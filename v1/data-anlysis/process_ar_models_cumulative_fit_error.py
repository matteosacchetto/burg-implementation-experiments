#!/usr/bin/env python3

import config
import statistics
import utils
from matplotlib import pyplot, use
import pandas
from math import sqrt
from os import path, makedirs, walk
import time
import json

specific_train_sizes = [2048]
lag_range = (1, 128)
show_confidence_intervals_flag = True
show_flag = False

def process_file(csv_filepath: str):
    csv_filename = path.splitext(path.basename(csv_filepath))[0]

    if not show_flag:
        use('Agg') # Non interactive mode of matplotlib to free memory
    
    # Read csv
    df = pandas.read_csv(csv_filepath)
    results = df.to_records()

    # Find categories
    categories = set()
    for result in results:
        category = utils.files.get_category(result['file'])
        categories.add(category)

    # Data structure to keep stats for each category
    results_by_categories = {}
    for category in categories:
        results_by_categories[category] = {}

    # Evaluate stats for each category
    for result in results:
        filename = result['file']
        category = utils.files.get_category(filename)
        # Ugly but needed to interpret the string as a list of dict
        stats = eval(result['results'])

        for stat in stats:
            train_size = stat['train_size']
            lag = stat['lag']
            ar_error = stat['ar_error']

            if(lag >= lag_range[0] and lag <= lag_range[1]):
            
                if train_size not in results_by_categories[category]:
                    results_by_categories[category][train_size] = {}

                if lag not in results_by_categories[category][train_size]:
                    results_by_categories[category][train_size][lag] = {
                        'ar_error': []
                    }

                # Append values
                results_by_categories[category][train_size][lag]['ar_error'] += ar_error

    # Keep track of the mean and variance
    mean_variance_by_categories = {}
    for category in categories:
        mean_variance_by_categories[category] = {}

    # Calculate average mean and variance by category
    for category, result_by_category in results_by_categories.items():
        # Loop through train sizes
        for train_size, lags in result_by_category.items():
            # Loop thrugh lags
            for lag, values in lags.items():
                # Evaluate mean, variance and std for error
                mean_error = statistics.mean(values['ar_error'])
                variance_error = statistics.variance(values['ar_error'])
                std_error = statistics.stdev(values['ar_error'])

                # Confidence interval rmse
                interval_error = 1.96 * (std_error/sqrt(len(values['ar_error'])))

                # Lower and higher limits of the interval rmse
                low_error = mean_error - interval_error
                high_error = mean_error + interval_error

                if train_size not in mean_variance_by_categories[category]:
                    mean_variance_by_categories[category][train_size] = {}

                mean_variance_by_categories[category][train_size][lag] = {
                    # error
                    'mean_error': mean_error,
                    'variance_error': variance_error,
                    'interval_error': interval_error,
                    'low_error': low_error,  # p-value it's a probability so it makes no sense a value < 0
                    'high_error': high_error  # same is true for a value > 1
                }

    # Create directory where to save the plots
    plot_root_dir = path.join(config.AR_MODEL_PLOT_DIRECTORY, csv_filename)
    makedirs(plot_root_dir, exist_ok=True)

    # Plot data grouped by category ERROR
    for category, elemets in mean_variance_by_categories.items():
        # Create a new figure
        pyplot.rc('font', size=16)
        pyplot.figure()
        
        for train_size, stats in elemets.items():
            # Filter on some train_size values if needed
            if(specific_train_sizes != None and specific_train_sizes != [] and train_size not in specific_train_sizes):
                continue

            # Plot with confidence intervals (error bars require the difference wrt to the mean => the interval)
            pyplot.errorbar(x=[el for el in stats], y=[el['mean_error'] for el in stats.values()], yerr=[[el['interval_error'] for el in stats.values()], [el['interval_error']
                            for el in stats.values()]] if show_confidence_intervals_flag else None, marker='o', elinewidth=1, capsize=5, zorder=2, label="train_size="+str(train_size))
            pyplot.xscale('log', base=2)  # Logaritmic x on base of 2
            # Replace powers with actual values
            pyplot.xticks(ticks=[el for el in stats],
                          labels=[el for el in stats])
            pyplot.xlabel('Lags', labelpad=10)
            pyplot.ylabel('Error', labelpad=10)

        pyplot.title(f'{category.capitalize()} - ERROR', pad=20)  # Title
        # Show legend only if the category is
        pyplot.gcf().set_tight_layout(True)  # Make sure that text is not cut out
        if show_flag:
            pyplot.show()
        else:
            pyplot.savefig(path.join(plot_root_dir, f'{category}-ERROR.png'), dpi=300)  # Save figure
        
        pyplot.close()


if __name__ == '__main__':
    for (root,dirs,files) in walk(path.join(config.ROOT_DIR, "results-2023-03-10_double"), topdown=True):
        for name in files:
            filepath = path.join(root, name)
            print(path.relpath(filepath, path.abspath('.')))
            process_file(filepath)
