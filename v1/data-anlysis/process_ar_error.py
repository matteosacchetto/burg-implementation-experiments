#!/usr/bin/env python3

import config
import statistics
import utils
from matplotlib import pyplot,  use
import pandas
from math import sqrt
from os import path, makedirs, walk
import time
import json
import gc

show_flag = False

def process_file(filepath: str):
    algo = '-'.join(path.splitext(path.basename(filepath))[0].split('-')[0:-1])
    if not show_flag:
        use('Agg') # Non interactive mode of matplotlib to free memory
        plot_root_dir = path.join(config.AR_MODEL_PLOT_DIRECTORY, 'error', path.splitext(path.basename(filepath))[0])
        makedirs(plot_root_dir, exist_ok=True)
    
    with open(filepath) as f:
        data = json.load(f)
        for el in data:
            train_size = el['train_size']
            order = el['lag']
            ar_ae = list(filter(lambda x: x != None, el['ar_ae'])) # Absolute errors
            ar_predictions = list(filter(lambda x: x!=None, el['prediction'])) # Actual predictions

            pyplot.rc('font', size=16)
            pyplot.figure(figsize=(10, 6))
            pyplot.plot(range(len(ar_predictions)), ar_predictions);
            pyplot.title(f'{algo} {train_size}-{order}', pad=20)  # Title

            pyplot.gcf().set_tight_layout(True)  # Make sure that text is not cut out
            if show_flag:
                pyplot.show()
            else:
                pyplot.savefig(path.join(plot_root_dir, f'{algo}-{train_size}-{order}.png'), dpi=300)  # Save figure            pyplot.close()
            pyplot.close()
            print(f'{algo}-{train_size}-{order}.png')


if __name__ == '__main__':
    for (root,dirs,files) in walk(path.join(config.ROOT_DIR, "results-2205"), topdown=True):
        for name in files:
            filepath = path.join(root, name)
            print(path.relpath(filepath, path.abspath('.')))
            process_file(filepath)
            gc.collect()
