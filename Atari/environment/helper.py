# coding=UTF8

def subimage_in_box(image, box):
    left, top, right, bottom = box
    return image[top:bottom, left:right]
