#!/usr/bin/env python3

import config
import statistics
import utils
import pandas
from math import sqrt
from os import path, makedirs, walk
import time
import json
import csv

specific_train_sizes = []  # [2048, 4096, 8192]
show_confidence_intervals_flag = True
show_multiplier_wrt_512_1 = False


def get_algo(file_path: str):
    return '-'.join(path.splitext(path.basename(file_path))[0].split('-')[0:-1])


def process_file(csv_filepath: str):
    # Read csv
    df = pandas.read_csv(csv_filepath)
    results = df.to_records()

    # Data structure to keep timing stats
    results_time = {}

    # Evaluate stats
    for result in results:
        b0 = eval(result['b0'])
        b1 = eval(result['b1'])
        # Ugly but needed to interpret the string as a list of dict
        stats = eval(result['results'])

        for stat in stats:
            train_size = stat['train_size']
            lag = stat['lag']
            ar_fit_time = stat['ar_fit_time']
            ar_predict_time = stat['ar_predict_time']

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

    # Calculate average mean and variance
    mean_variance_time = {}
    for train_size, lags in results_time.items():
        # Loop thrugh lags
        for lag, values in lags.items():
            # Evaluate mean, variance and std for fit time
            # Convert to ms
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
            ar_predict_times = [
                x / 1e6 for x in values['ar_predict_time']]  # Convert to ms
            mean_predict_time = statistics.mean(ar_predict_times)
            variance_predict_time = statistics.variance(ar_predict_times)
            std_predict_time = statistics.stdev(ar_predict_times)

            # Confidence interval predict time
            interval_predict_time = 1.96 * \
                (std_predict_time/sqrt(len(ar_predict_times)))

            # Lower and higher limits of the interval predict time
            low_predict_time = mean_predict_time - interval_predict_time
            high_predict_time = mean_predict_time + interval_predict_time

            if train_size not in mean_variance_time:
                mean_variance_time[train_size] = {}

            mean_variance_time[train_size][lag] = {
                # fit_time
                'mean_fit_time': mean_fit_time,
                'variance_fit_time': variance_fit_time,
                'std_fit_time': std_fit_time,
                'interval_fit_time': interval_fit_time,
                # p-value it's a probability so it makes no sense a value < 0
                'low_fit_time': low_fit_time,
                'high_fit_time': high_fit_time,  # same is true for a value > 1

                # predict_time
                'mean_predict_time': mean_predict_time,
                'variance_predict_time': variance_predict_time,
                'std_predict_time': std_predict_time,
                'interval_predict_time': interval_predict_time,
                # p-value it's a probability so it makes no sense a value < 0
                'low_predict_time': low_predict_time,
                'high_predict_time': high_predict_time  # same is true for a value > 1
            }

    # Save data
    return mean_variance_time


def flat_map(f, xs): return (y for ys in xs for y in f(ys))


def process_aggregated_results(results: list):
    header = ['algo']
    header.extend(list(flat_map(lambda el: list(map(
        lambda lagel: f'{el[0]}-{lagel[0]}', el[1].items())), results[0]['stats'].items())))

    makedirs(config.TIME_STATS_DIR, exist_ok=True)

    with open(path.join(config.TIME_STATS_DIR, 'fit_time.csv'), mode='w') as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=header)
        writer.writeheader()
        for result in results:
            row = {x[0]: x[1] for x in list(flat_map(lambda x: x, [
                                            [(f'{el[0]}-{lagel[0]}',  f'{lagel[1]["mean_fit_time"]}±{lagel[1]["interval_fit_time"]}') for lagel in el[1].items()] for el in result['stats'].items()]))}
            row['algo'] = result['algo']
            writer.writerow(row)

    with open(path.join(config.TIME_STATS_DIR, 'predict_time.csv'), mode='w') as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=header)
        writer.writeheader()
        for result in results:
            row = {x[0]: x[1] for x in list(flat_map(lambda x: x, [
                                            [(f'{el[0]}-{lagel[0]}',  f'{lagel[1]["mean_predict_time"]}±{lagel[1]["interval_predict_time"]}') for lagel in el[1].items()] for el in result['stats'].items()]))}
            row['algo'] = result['algo']
            writer.writerow(row)


if __name__ == '__main__':
    results = list()
    for (root, dirs, files) in walk(path.join(config.ROOT_DIR, "results-2023-02-21_double"), topdown=True):
        for name in files:
            algo = get_algo(name)
            filepath = path.join(root, name)
            print(path.relpath(filepath, path.abspath('.')))
            stats = process_file(filepath)
            if show_multiplier_wrt_512_1:
                row = {x[0]: x[1] for x in list(flat_map(lambda x: x, [
                                                [(f'{el[0]}-{lagel[0]}',  lagel[1]["mean_fit_time"]) for lagel in el[1].items()] for el in stats.items()]))}
                for el in row.items():
                    print(el[1] / row['512-1'])
            results.append({
                'algo': algo,
                'stats': stats
            })

    process_aggregated_results(results)
