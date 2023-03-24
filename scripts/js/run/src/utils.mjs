/**
 *
 * @param {string} cmd
 *
 * @return {string[]} split command
 */
export const command = (cmd) => {
  return cmd
    .split(/(["'].*["'])/)
    .filter((el) => el !== '')
    .flatMap((el) =>
      !(el.startsWith('"') || el.startsWith("'"))
        ? el.split(' ').filter((el) => el !== '')
        : el
    ).map(el => el.startsWith('"') || el.startsWith("'") ? el.substring(1, el.length - 1) : el);
};
