import { createCommand } from './command.mjs';
import { name } from './config.mjs';
import burgCommand from './burg.mjs';
import errorCommand from './error.mjs';

const program = createCommand(name, '');

// Add sub-programs
program.addCommand(burgCommand);
program.addCommand(errorCommand);

// Parse arguments
program.parse(process.argv);
