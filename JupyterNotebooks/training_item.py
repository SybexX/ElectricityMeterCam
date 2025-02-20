from PIL import Image, ImageEnhance, ImageOps, ImageChops
import numpy as np
import matplotlib.pyplot as plt

class TrainingItem:
    image = None
    y = None
    
    def __init__(self):
        self.y = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    
    def show(self):
        print(self.y)
        plt.imshow(self.image, vmin=0, vmax=255)
        plt.show()

    def get_max_class_idx(self):
        return np.argmax(self.y)