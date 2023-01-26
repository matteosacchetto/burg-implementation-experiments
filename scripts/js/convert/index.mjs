import { walk, ffmpeg } from './lib.mjs';
import { program } from 'commander';
import { lstat } from 'node:fs/promises';

program
  .option('-i, --input <dir_or_file>', 'input file or directory', '.')
  .option('-s, --sr <sample_rate>', 'sample rate', 44100)
  .option('-n, --num-channels <num>', 'number of channels', 1)
  .option('-f, --sample-format <format>', 'sample format', 's16')
  .option('--rm', 'delete source files', false);

program.parse();

const { input, sr, numChannels: num_channels, sampleFormat: sample_fmt, rm: remove_src_file } = program.opts();

const list = [];
try {
  const stat = await lstat(input);
  if (stat.isDirectory()) {
    list.push(...(await walk(input)));
  } else if (stat.isFile()) {
    list.push(input);
  }
} catch (e) {
  if (e instanceof Error) {
    console.error(e.message);
  } else {
    console.error(e);
  }
}

const filtered_list = list.filter((el) =>
  /(wav)|(mp3)|(ogg)$/.test(el.toLocaleLowerCase().split('.').at(-1))
);

if (filtered_list.length === 0) {
  console.log('No file to process...');
}
else {
  let i = 0;
  for (const el of filtered_list) {
    await ffmpeg(el, {
      sr,
      num_channels,
      sample_fmt,
      remove_src_file
    }) && i++;
  }

  console.log(`${i} file processed`);
}

