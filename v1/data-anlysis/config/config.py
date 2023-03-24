from os import path
from typing import Final

ROOT_DIR = path.abspath(path.join(path.dirname(path.abspath(__file__)), '..', '..')) # This is your Project Root
PLOT_DIR = path.join(ROOT_DIR, 'plots')

AR_MODEL_PLOT_DIRECTORY = path.join(PLOT_DIR, 'ar')

TIME_STATS_DIR = path.join(ROOT_DIR, 'time_stats')