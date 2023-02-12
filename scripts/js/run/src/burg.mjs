import { exec } from "./exec.mjs";

import { Option, program } from "commander";
import { mkdir } from "node:fs/promises";
import { openSync } from "node:fs";
import { join } from "node:path";
import { createCommand } from "./command.mjs";

const name = "burg";
const description = "Run the burg tests";

const burgCommand = createCommand(name, description)
  .option("-o, --output <dir>", "output directory")
  .addOption(
    new Option("-d, --data-type <type>", "data type")
      .choices(["D", "LD"])
      .default("D")
  );

burgCommand.action(async (options) => {
  const { output, dataType } = options;

  if (output) await mkdir(output, { recursive: true });
  else output = ".";

  const DATA_TYPE =
    dataType == "D" ? "DOUBLE" : dataType == "LD" ? "LONG_DOUBLE" : "DOUBLE";

  const commands = [
    // No flags
    {
      cmd_name: "burg-basic",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DBURG=BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DBURG=OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den-sqrt",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DBURG=OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-basic",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DBURG=COMP_BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DBURG=COMP_OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den-sqrt",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DBURG=COMP_OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },

    // Native
    {
      cmd_name: "burg-basic-native",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBURG=BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den-native",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBURG=OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den-sqrt-native",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBURG=OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-basic-native",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBURG=COMP_BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den-native",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBURG=COMP_OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den-sqrt-native",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DBURG=COMP_OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },

    // Fast math
    {
      cmd_name: "burg-basic-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DFAST_MATH=ON -DBURG=BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DFAST_MATH=ON -DBURG=OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den-sqrt-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DFAST_MATH=ON -DBURG=OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-basic-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DFAST_MATH=ON -DBURG=COMP_BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DFAST_MATH=ON -DBURG=COMP_OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den-sqrt-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DFAST_MATH=ON -DBURG=COMP_OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },

    // Native fast-math
    {
      cmd_name: "burg-basic-native-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DFAST_MATH=ON -DBURG=BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den-native-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DFAST_MATH=ON -DBURG=OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "burg-optimized-den-sqrt-native-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DFAST_MATH=ON -DBURG=OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-basic-native-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DFAST_MATH=ON -DBURG=COMP_BASIC -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den-native-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DFAST_MATH=ON -DBURG=COMP_OPT_DEN -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
    {
      cmd_name: "compensated-burg-optimized-den-sqrt-native-fast_math",
      compile: [
        `cmake -B tmp_build -DCMAKE_BUILD_TYPE=Release -DNATIVE=ON -DFAST_MATH=ON -DBURG=COMP_OPT_DEN_SQRT -DDATA_TYPE=${DATA_TYPE}`,
        "cmake --build tmp_build -j 4",
      ],
      run: "./tmp_build/burg",
      clean: "rm -r tmp_build",
    },
  ];

  try {
    for (const command of commands) {
      // Compile
      for (const compile_cmd of command.compile) {
        await exec(compile_cmd);
      }

      // Run
      await exec(command.run, {
        stdio: [
          0,
          openSync(join(output, `${command.cmd_name}-${Date.now()}.csv`), "w"),
          2,
        ],
      });

      // Clean
      await exec(command.clean);
    }
  } catch (e) {
    console.error(`${e.message}`);
  }
});

export default burgCommand;
