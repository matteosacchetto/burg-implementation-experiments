import chalk from 'chalk';
import { spawn } from 'child_process';
import { command } from './utils.mjs';

/**
 *
 * @param {string[]|string} command
 */
export async function exec(cmd, { stdio } = { stdio: [0, 1, 2] }) {
  if (typeof cmd === 'string') {
    cmd = command(cmd);
  }

  const child = spawn(cmd[0], cmd.slice(1), {
    stdio: stdio,
  });

  const exitCode = await new Promise((resolve) => {
    child.on('close', resolve);
  });

  if (exitCode) {
    throw new Error(
      process.stderr.isTTY
        ? `${chalk.red(`[EXIT_CODE]`)}: ${chalk.yellow(exitCode)}`
        : `[EXIT_CODE]: ${exitCode}`
    );
  }
}
