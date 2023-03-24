import { join } from 'node:path';
import { opendir, rm } from 'node:fs/promises';
import { promisify } from 'node:util';
import { exec as execCallback } from 'node:child_process';
import ora from 'ora';
import chalk from 'chalk';
import { existsSync, fstat } from 'node:fs';

const exec = promisify(execCallback);

/**
 * Walk the specified directory
 *
 * @param {string} dir
 * @returns {Array<string>}
 */
export const walk = async (dir) => {
  const entrties = await opendir(dir);
  const list = [];
  for await (const entry of entrties) {
    const entryPath = join(dir, entry.name);
    if (entry.isDirectory()) {
      list.push(...(await walk(entryPath)));
    } else if (entry.isFile()) {
      list.push(entryPath);
    }
  }

  return list;
};

/**
 * @typedef {"u8" | "s16" | "s32" | "s64" | "flt" | "dbl" | "u8p | "s16p" | "s32p" | "s64p" | "fltp" | "dblp"} sample_formats
 *
 * @param {string} file
 * @param {Object} opt
 * @param {number} opt.sr
 * @param {number} opt.num_channels
 * @param {sample_formats} opt.sample_fmt
 * @param {number} opt.remove_src_file
 */
export const ffmpeg = async (
  file,
  {
    sr = 44100,
    num_channels = 1,
    sample_fmt = 's16',
    remove_src_file = false,
  } = {}
) => {
  if (file.split('.')[0].endsWith(`_${sr}_${num_channels}`)) {
    return false;
  }

  const spinner = ora(`${chalk.bold(file)}`);
  try {
    const output_file = `${file.split('.')[0]}_${sr}_${num_channels}.wav`;

    if (existsSync(output_file)) {
      if (remove_src_file) {
        spinner.succeed(
          `${chalk.gray(file)} 🠒 ${chalk.bold.cyan(output_file)}`
        );
      } else {
        // spinner.succeed(`${chalk.gray(file)} ${chalk.gray(`[SKIPPED]`)}`);
        return false;
      }
    } else {
      await exec(
        `ffmpeg -i ${file} -ar ${sr} -ac ${num_channels} -sample_fmt ${sample_fmt} -y ${output_file}`
      );

      if (remove_src_file) {
        spinner.succeed(
          `${chalk.gray(file)} 🠒 ${chalk.bold.cyan(output_file)}`
        );
      } else {
        spinner.succeed(`${chalk.bold.cyan(file)}`);
      }
    }

    if (remove_src_file) {
      await rm(file);
    }

    return true;
  } catch (e) {
    spinner.fail();
    console.error(e);
  }

  return false;
};
