import { version } from './config.mjs';
import { uncapitalize } from './string-utils.mjs';
import { Command } from 'commander';

export const createCommand = (name, description) => {
  // Initialize CLI (name , description)
  const program = new Command(name);
  program.description(description);

  // Set version
  program.version(version, '-v, --version');

  // Uncapitalize help for sub commands
  program.configureHelp({
    subcommandDescription: (cmd) => {
      return uncapitalize(cmd.description());
    },
  });

  // Show suggestions after a error
  program.showSuggestionAfterError();

  // Return the new Command
  return program;
};
