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
import numpy

show_flag = False
data_type = numpy.double
num_pred = 2560
specific_train_sizes = []

names_to_names = {
    'burg-basic': 'Burg\'s method',
    'burg-optimized-den': 'Denominator optimization',
    'burg-optimized-den-sqrt': 'Hybrid denominator',
    'compensated-burg-basic': 'Burg\'s method (compensated)',
    'compensated-burg-optimized-den': 'Den. opt. (compensated)',
    'compensated-burg-optimized-den-sqrt': 'Hybrid den. (compensated)'
}

if not show_flag:
    use('Agg')

def process_file(filepath: str):
    algo = '-'.join(path.splitext(path.basename(filepath))[0].split('-')[0:-1])
    if not show_flag:
        use('Agg') # Non interactive mode of matplotlib to free memory
        plot_root_dir = path.join(config.AR_MODEL_PLOT_DIRECTORY, 'error', path.splitext(path.basename(filepath))[0])
        makedirs(plot_root_dir, exist_ok=True)
    
    results = {}

    with open(filepath) as f:
        data = json.load(f)
        for el in data:
            train_size = el['train_size']
            order = el['lag']

            if train_size not in results:
                results[train_size] = {}
            if order not in results[train_size]:
                results[train_size][order] = {
                    'mae': 2 # Since the range of values is between 0 and 0.7, we use 2 to indicate divergence
                }
            
            ar_ae = numpy.array(list(filter(lambda x: x != None, el['ar_ae'])), dtype=data_type) # Absolute errors

            if(len(ar_ae) != num_pred):
                continue
            
            mae = (numpy.sum(ar_ae) / num_pred)

            if(abs(mae) > 2):
                continue
            else:
                results[train_size][order]['mae'] = mae

    return algo, results
            


if __name__ == '__main__':
    results = {}
    for (root,dirs,files) in walk(path.join(config.ROOT_DIR, "results-2000_double-2"), topdown=True):
        for name in files:
            filepath = path.join(root, name)
            print(path.relpath(filepath, path.abspath('.')))
            algo, tmp = process_file(filepath)
            results[algo] = tmp
            gc.collect()

    if not show_flag:
        # Create directory where to save the plots
        plot_root_dir = path.join(config.AR_MODEL_PLOT_DIRECTORY, 'prediction_errors')
        makedirs(plot_root_dir, exist_ok=True)

    # Plot data grouped by category ERROR
    it = 0
    for algo, elements in results.items():
        # Create a new figure
        pyplot.rc('font', size=16)
        pyplot.figure()
        
        for train_size, stats in elements.items():
            # Filter on some train_size values if needed
            if(specific_train_sizes != None and specific_train_sizes != [] and train_size not in specific_train_sizes):
                continue

            # Plot with confidence intervals (error bars require the difference wrt to the mean => the interval)
            pyplot.errorbar(x=[el for el in stats], y=[el['mae'] for el in stats.values()], marker='o', elinewidth=1, capsize=5, zorder=2, label="train_size="+str(train_size))
            pyplot.xscale('log', base=2)  # Logaritmic x on base of 2
            # Replace powers with actual values
            pyplot.xticks(ticks=[el for el in stats],
                          labels=[el for el in stats])
            pyplot.xlabel('Order', labelpad=10)
            pyplot.ylabel('MAE', labelpad=10)

        ax = pyplot.gca()
        handles, labels = ax.get_legend_handles_labels()

        pyplot.title(f'{names_to_names[algo] if algo in names_to_names else algo}', pad=20)  # Title
        pyplot.ylim([-0.05, 0.65])
        # Show legend only if the category is
        pyplot.gcf().set_tight_layout(True)  # Make sure that text is not cut out
        if show_flag:
            pyplot.show()
        else:
            pyplot.savefig(path.join(plot_root_dir, f'{algo}-pe.png'), dpi=300)  # Save figure
        
        pyplot.close()

        if(it == 0):
            # Save legend
            ncol = 3

            pyplot.figure(figsize=(15, 3))
            pyplot.axis(False)
            handles = [x for y in [handles[p::ncol] for p in range(ncol)] for x in y]
            labels = [x for y in [labels[p::ncol] for p in range(ncol)] for x in y]
            pyplot.gcf().set_tight_layout(True)
            pyplot.legend(handles, labels, loc="center", bbox_to_anchor=(0.5, 0.5), mode="extend", ncol=ncol, prop={"size":20})
            pyplot.savefig(path.join(plot_root_dir,
                                f'legend.png'), dpi=300)
            pyplot.close()
        
        it+=1
