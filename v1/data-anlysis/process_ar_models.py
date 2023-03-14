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

specific_train_sizes = []  # [2048, 4096, 8192]
show_confidence_intervals_flag = True
show_flag = False
group1_categories = ['violin', 'drums', 'piano']


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

    # Data structure to keep timing stats
    results_time = {}

    # Data structure to keep benchmarks for each category
    benchmarks_by_category = {}
    for category in categories:
        benchmarks_by_category[category] = {
            'b0': {
                'mae': [],
                'rmse': []
            },
            'b1': {
                'mae': [],
                'rmse': []
            },
        }

    # Evaluate stats for each category
    for result in results:
        filename = result['file']
        category = utils.files.get_category(filename)
        b0 = eval(result['b0'])
        b1 = eval(result['b1'])
        # Ugly but needed to interpret the string as a list of dict
        stats = eval(result['results'])

        for stat in stats:
            train_size = stat['train_size']
            lag = stat['lag']
            ar_mae = stat['ar_mae']
            ar_rmse = stat['ar_rmse']
            ar_fit_time = stat['ar_fit_time']
            ar_predict_time = stat['ar_predict_time']

            if train_size not in results_by_categories[category]:
                results_by_categories[category][train_size] = {}

            if lag not in results_by_categories[category][train_size]:
                results_by_categories[category][train_size][lag] = {
                    'ar_mae': [],
                    'ar_rmse': []
                }

            # Append values
            results_by_categories[category][train_size][lag]['ar_mae'] += ar_mae
            results_by_categories[category][train_size][lag]['ar_rmse'] += ar_rmse

            if train_size not in results_time:
                results_time[train_size] = {}

            if lag not in results_time[train_size]:
                results_time[train_size][lag] = {
                    'ar_fit_time': [],
                    'ar_predict_time': []
                }
            
            # Append values
            results_time[train_size][lag]['ar_fit_time'] += ar_fit_time
            results_time[train_size][lag]['ar_predict_time'] += ar_predict_time


        benchmarks_by_category[category]['b0']['mae'] += b0['mae']
        benchmarks_by_category[category]['b0']['rmse'] += b0['rmse']

        benchmarks_by_category[category]['b1']['mae'] += b1['mae']
        benchmarks_by_category[category]['b1']['rmse'] += b1['rmse']

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
                # Evaluate mean, variance and std for mae
                mean_mae = statistics.mean(values['ar_mae'])
                variance_mae = statistics.variance(values['ar_mae'])
                std_mae = statistics.stdev(values['ar_mae'])

                # Confidence interval mae
                interval_mae = 1.96 * (std_mae/sqrt(len(values['ar_mae'])))

                # Lower and higher limits of the interval mae
                low_mae = mean_mae - interval_mae
                high_mae = mean_mae + interval_mae

                # Evaluate mean, variance and std for rmse
                mean_rmse = statistics.mean(values['ar_rmse'])
                variance_rmse = statistics.variance(values['ar_rmse'])
                std_rmse = statistics.stdev(values['ar_rmse'])

                # Confidence interval rmse
                interval_rmse = 1.96 * (std_rmse/sqrt(len(values['ar_rmse'])))

                # Lower and higher limits of the interval rmse
                low_rmse = mean_rmse - interval_rmse
                high_rmse = mean_rmse + interval_rmse

                if train_size not in mean_variance_by_categories[category]:
                    mean_variance_by_categories[category][train_size] = {}

                mean_variance_by_categories[category][train_size][lag] = {
                    # mae
                    'mean_mae': mean_mae,
                    'variance_mae': variance_mae,
                    'interval_mae': interval_mae,
                    'low_mae': low_mae,  # p-value it's a probability so it makes no sense a value < 0
                    'high_mae': high_mae,  # same is true for a value > 1

                    # rmse
                    'mean_rmse': mean_rmse,
                    'variance_rmse': variance_rmse,
                    'interval_rmse': interval_rmse,
                    'low_rmse': low_rmse,  # p-value it's a probability so it makes no sense a value < 0
                    'high_rmse': high_rmse  # same is true for a value > 1
                }

    mean_benchmarks_by_categories = {}
    for category in categories:
        mean_benchmarks_by_categories[category] = {}

    for category, benchmarks in benchmarks_by_category.items():
        for benchmark, values in benchmarks.items():
            # Evaluate mean, variance and std for mae
            mean_mae = statistics.mean(values['mae'])
            variance_mae = statistics.variance(values['mae'])
            std_mae = statistics.stdev(values['mae'])

            # Confidence interval mae
            interval_mae = 1.96 * (std_mae/sqrt(len(values['mae'])))

            # Lower and higher limits of the interval mae
            low_mae = mean_mae - interval_mae
            high_mae = mean_mae + interval_mae

            # Evaluate mean, variance and std for rmse
            mean_rmse = statistics.mean(values['rmse'])
            variance_rmse = statistics.variance(values['rmse'])
            std_rmse = statistics.stdev(values['rmse'])

            # Confidence interval rmse
            interval_rmse = 1.96 * (std_rmse/sqrt(len(values['rmse'])))

            # Lower and higher limits of the interval rmse
            low_rmse = mean_rmse - interval_rmse
            high_rmse = mean_rmse + interval_rmse

            mean_benchmarks_by_categories[category][benchmark] = {
                # mae
                'mean_mae': mean_mae,
                'variance_mae': variance_mae,
                'interval_mae': interval_mae,
                'low_mae': low_mae,  # p-value it's a probability so it makes no sense a value < 0
                'high_mae': high_mae,  # same is true for a value > 1

                # rmse
                'mean_rmse': mean_rmse,
                'variance_rmse': variance_rmse,
                'interval_rmse': interval_rmse,
                'low_rmse': low_rmse,  # p-value it's a probability so it makes no sense a value < 0
                'high_rmse': high_rmse  # same is true for a value > 1
            }
    
    mean_variance_time = {}
    for train_size, lags in results_time.items():
        # Loop thrugh lags
        for lag, values in lags.items():
            # Evaluate mean, variance and std for fit time
            ar_fit_times = [x / 1e6 for x in values['ar_fit_time']]
            mean_fit_time = statistics.mean(ar_fit_times)
            variance_fit_time = statistics.variance(ar_fit_times)
            std_fit_time = statistics.stdev(ar_fit_times)

            # Confidence interval fit time
            interval_fit_time = 1.96 * (std_fit_time/sqrt(len(ar_fit_times)))

            # Lower and higher limits of the interval fit time
            low_fit_time = mean_fit_time - interval_fit_time
            high_fit_time = mean_fit_time + interval_fit_time

            # Evaluate mean, variance and std for predict time
            ar_predict_times = [x / 1e6 for x in values['ar_predict_time']]
            mean_predict_time = statistics.mean(ar_predict_times)
            variance_predict_time = statistics.variance(ar_predict_times)
            std_predict_time = statistics.stdev(ar_predict_times)

            # Confidence interval predict time
            interval_predict_time = 1.96 * (std_predict_time/sqrt(len(ar_predict_times)))

            # Lower and higher limits of the interval predict time
            low_predict_time = mean_predict_time - interval_predict_time
            high_predict_time = mean_predict_time + interval_predict_time

            if train_size not in mean_variance_time:
                mean_variance_time[train_size] = {}

            mean_variance_time[train_size][lag] = {
                # fit_time
                'mean_fit_time': mean_fit_time,
                'variance_fit_time': variance_fit_time,
                'interval_fit_time': interval_fit_time,
                'low_fit_time': low_fit_time,  # p-value it's a probability so it makes no sense a value < 0
                'high_fit_time': high_fit_time,  # same is true for a value > 1

                # predict_time
                'mean_predict_time': mean_predict_time,
                'variance_predict_time': variance_predict_time,
                'interval_predict_time': interval_predict_time,
                'low_predict_time': low_predict_time,  # p-value it's a probability so it makes no sense a value < 0
                'high_predict_time': high_predict_time  # same is true for a value > 1
            }

    # Create directory where to save the plots
    plot_root_dir = path.join(config.AR_MODEL_PLOT_DIRECTORY, csv_filename)
    makedirs(plot_root_dir, exist_ok=True)
    
    # Plot data grouped by category MAE
    for category, elemets in mean_variance_by_categories.items():
        # Create a new figure
        pyplot.rc('font', size=16)
        pyplot.figure()
        pyplot.axhline(mean_benchmarks_by_categories[category]['b0']['mean_mae'],
                       color='grey', lw=2, zorder=1, label='silence substitution')
        if show_confidence_intervals_flag:
            pyplot.axhspan(mean_benchmarks_by_categories[category]['b0']['low_mae'],
                           mean_benchmarks_by_categories[category]['b0']['high_mae'], facecolor='grey', alpha=0.5)
        pyplot.axhline(mean_benchmarks_by_categories[category]['b1']['mean_mae'],
                       color='orange', lw=2, zorder=1, label='pattern replication')
        if show_confidence_intervals_flag:
            pyplot.axhspan(mean_benchmarks_by_categories[category]['b1']['low_mae'],
                           mean_benchmarks_by_categories[category]['b1']['high_mae'], facecolor='orange', alpha=0.5)
        for train_size, stats in elemets.items():
            # Filter on some train_size values if needed
            if(specific_train_sizes != None and specific_train_sizes != [] and train_size not in specific_train_sizes):
                continue

            # Plot with confidence intervals (error bars require the difference wrt to the mean => the interval)
            pyplot.errorbar(x=[el for el in stats], y=[el['mean_mae'] for el in stats.values()], yerr=[[el['interval_mae'] for el in stats.values()], [el['interval_mae']
                            for el in stats.values()]] if show_confidence_intervals_flag else None, marker='o', elinewidth=1, capsize=5, zorder=2, label="train_size="+str(train_size))
            pyplot.xscale('log', base=2)  # Logaritmic x on base of 2
            # Replace powers with actual values
            pyplot.xticks(ticks=[el for el in stats],
                          labels=[el for el in stats])
            pyplot.xlabel('Order', labelpad=10)
            pyplot.ylabel('MAE', labelpad=10)

        ax = pyplot.gca()
        handles, labels = ax.get_legend_handles_labels()

        pyplot.title(f'{category.capitalize()} - MAE', pad=20)  # Title

        if category in group1_categories:
            pyplot.ylim(0, 0.25)
        else:
            pyplot.ylim(0, 0.25)
        pyplot.gcf().set_tight_layout(True)  # Make sure that text is not cut out
        if show_flag:
            pyplot.show()
        else:
            pyplot.savefig(path.join(plot_root_dir,
                           f'{category}-MAE.png'), dpi=300)  # Save figure
        
        pyplot.close()

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
        

    # Plot data grouped by category RMSE
    for category, elemets in mean_variance_by_categories.items():
        # Create a new figure
        pyplot.rc('font', size=16)
        pyplot.figure()
        pyplot.axhline(mean_benchmarks_by_categories[category]['b0']['mean_rmse'],
                       color='grey', lw=2, zorder=1, label='silence substitution')
        if show_confidence_intervals_flag:
            pyplot.axhspan(mean_benchmarks_by_categories[category]['b0']['low_rmse'],
                           mean_benchmarks_by_categories[category]['b0']['high_rmse'], facecolor='grey', alpha=0.5)
        pyplot.axhline(mean_benchmarks_by_categories[category]['b1']['mean_rmse'],
                       color='orange', lw=2, zorder=1, label='pattern replication')
        if show_confidence_intervals_flag:
            pyplot.axhspan(mean_benchmarks_by_categories[category]['b1']['low_rmse'],
                           mean_benchmarks_by_categories[category]['b1']['high_rmse'], facecolor='orange', alpha=0.5)

        for train_size, stats in elemets.items():
            # Filter on some train_size values if needed
            if(specific_train_sizes != None and specific_train_sizes != [] and train_size not in specific_train_sizes):
                continue

            # Plot with confidence intervals (error bars require the difference wrt to the mean => the interval)
            pyplot.errorbar(x=[el for el in stats], y=[el['mean_rmse'] for el in stats.values()], yerr=[[el['interval_rmse'] for el in stats.values()], [el['interval_rmse']
                            for el in stats.values()]] if show_confidence_intervals_flag else None, marker='o', elinewidth=1, capsize=5, zorder=2, label="train_size="+str(train_size))
            pyplot.xscale('log', base=2)  # Logaritmic x on base of 2
            # Replace powers with actual values
            pyplot.xticks(ticks=[el for el in stats],
                          labels=[el for el in stats])
            pyplot.xlabel('Order', labelpad=10)
            pyplot.ylabel('RMSE', labelpad=10)

        pyplot.title(f'{category.capitalize()} - RMSE', pad=20)  # Title
        # Show legend only if the category is
        if category in group1_categories:
            pyplot.ylim(0, 0.3)
        else:
            pyplot.ylim(0, 0.3)
        pyplot.gcf().set_tight_layout(True)  # Make sure that text is not cut out
        if show_flag:
            pyplot.show()
        else:
            pyplot.savefig(path.join(plot_root_dir, f'{category}-RMSE.png'), dpi=300)  # Save figure
        
        pyplot.close()

    # Create a new figure
    pyplot.rc('font', size=16)
    pyplot.figure()

    for train_size, stats in mean_variance_time.items():
        # Filter on some train_size values if needed
        if(specific_train_sizes != None and specific_train_sizes != [] and train_size not in specific_train_sizes):
            continue

        # Plot with confidence intervals (error bars require the difference wrt to the mean => the interval)
        pyplot.errorbar(x=[el for el in stats], y=[el['mean_fit_time'] for el in stats.values()], yerr=[[el['interval_fit_time'] for el in stats.values()], [el['interval_fit_time']
                        for el in stats.values()]] if show_confidence_intervals_flag else None, marker='o', elinewidth=1, capsize=5, zorder=2, label="train_size="+str(train_size))
        pyplot.xscale('log', base=2)  # Logaritmic x on base of 2
        # Replace powers with actual values
        pyplot.xticks(ticks=[el for el in stats],
                        labels=[el for el in stats])
        pyplot.xlabel('Order', labelpad=10)
        pyplot.ylabel('Time (ms)', labelpad=10)

    pyplot.title(f'AR fit time', pad=20)  # Title
    # Show legend only if the category is
    pyplot.legend(loc='upper left')  # Show legend
    pyplot.gcf().set_tight_layout(True)  # Make sure that text is not cut out
    if show_flag:
        pyplot.show()
    else:
        pyplot.savefig(path.join(plot_root_dir, 'fit-time.png'), dpi=300)  # Save figure

    pyplot.close()

    # Create a new figure
    pyplot.rc('font', size=16)
    pyplot.figure()

    train_size = list(mean_variance_time.keys())[0]
    stats = mean_variance_time[train_size]

    # Plot with confidence intervals (error bars require the difference wrt to the mean => the interval)
    pyplot.errorbar(x=[el for el in stats], y=[el['mean_predict_time'] for el in stats.values()], yerr=[[el['interval_predict_time'] for el in stats.values()], [el['interval_predict_time']
                    for el in stats.values()]] if show_confidence_intervals_flag else None, marker='o', elinewidth=1, capsize=5, zorder=2, label="train_size="+str(train_size))
    pyplot.xscale('log', base=2)  # Logaritmic x on base of 2
    # Replace powers with actual values
    pyplot.xticks(ticks=[el for el in stats],
                    labels=[el for el in stats])
    pyplot.xlabel('Order', labelpad=10)
    pyplot.ylabel('Time (ms)', labelpad=10)

    pyplot.title(f'AR predict time', pad=20)  # Title
    # Show legend only if the category is
    # pyplot.legend(loc='upper left')  # Show legend
    pyplot.gcf().set_tight_layout(True)  # Make sure that text is not cut out
    if show_flag:
        pyplot.show()
    else:
        pyplot.savefig(path.join(plot_root_dir, 'predict-time.png'), dpi=300)  # Save figure

    pyplot.close()


if __name__ == '__main__':
    for (root,dirs,files) in walk(path.join(config.ROOT_DIR, "results-2023-03-14_double"), topdown=True):
        for name in files:
            filepath = path.join(root, name)
            print(path.relpath(filepath, path.abspath('.')))
            process_file(filepath)
