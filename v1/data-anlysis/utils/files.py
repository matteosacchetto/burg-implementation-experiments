import os

def get_category(filename):
    return filename.split(os.sep)[-2]