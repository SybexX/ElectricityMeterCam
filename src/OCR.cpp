#include "OCR.h"
#include <Arduino.h>

constexpr int tensor_pool_size = 64 * 1024;

OCR::OCR(const void* model, const int inputWidth, const int inputHeight) :
    _inputWidth(inputWidth),
    _inputHeight(inputHeight),
    _interpreter(nullptr),
    _input(nullptr),
    _output(nullptr)
{
    // Load the model
	Serial.println("Loading Tensorflow model....");
	_model = tflite::GetModel(model);
	Serial.println("model loaded!");

	// Define ops resolver and error reporting
	static tflite::ops::micro::AllOpsResolver resolver;

	static tflite::ErrorReporter* error_reporter;
	static tflite::MicroErrorReporter micro_error;
	error_reporter = &micro_error;

	// Instantiate the interpreter 
    Serial.println("Allocating memory pool");
    _tensorMemoryPool = (uint8_t*)malloc(tensor_pool_size);
    if (_tensorMemoryPool == nullptr)
    {
        Serial.print("failed to allocate tensor pool");
        return;
    }

	static tflite::MicroInterpreter static_interpreter(
		_model, resolver, _tensorMemoryPool, tensor_pool_size, error_reporter
	);

	_interpreter = &static_interpreter;

	// Allocate the the model's tensors in the memory pool that was created.
	Serial.println("Allocating tensors to memory pool");
	if(_interpreter->AllocateTensors() != kTfLiteOk) {
		Serial.println("There was an error allocating the memory...ooof");
		return;
	}

	// Define input and output nodes
	_input = _interpreter->input(0);
	_output = _interpreter->output(0);
	Serial.println("Starting inferences... ! ");
}

OCR::~OCR()
{
    if (_tensorMemoryPool != nullptr)
    {
        free(_tensorMemoryPool);
    }  
}

int OCR::PredictDigit(const dl_matrix3du_t* frame, const int rectX, const int rectY, const int rectWidth, const int rectHeight)
{
    ImageUtils::GetNormalizedPixels(
        frame, 
        rectX,
        rectY,
        rectWidth,
        rectHeight,
        _input->data.f,
        _inputWidth,
        _inputHeight);

    // Run inference on the input data
    if(_interpreter->Invoke() != kTfLiteOk)
    {
        Serial.println("There was an error invoking the interpreter!");
        return -1;
    }

    float bestConv = 0.0;
    int bestMatch = -1;
    for (int i = 0; i < 10; i++)
    {        
        Serial.println(String(i) + " -> " + _output->data.f[i] * 100);
        if (_output->data.f[i] > bestConv)
        {
            bestConv = _output->data.f[i];
            bestMatch = i;
        }
    }
    return bestMatch;
}