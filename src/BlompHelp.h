#pragma once

#include <string>

namespace HelpText
{

// ---------- GENERAL ----------

static const char* general =
R"(Usage:
  blomp [mode] [options] [inFile]

Modes:
  help         View this help.
  enc          Convert an image file to a blomp file.
  dec          Convert a blomp file to an image file.
  denc         Shortcut for running 'enc' and 'dec'. Doesn't save the *.blp file.
  comp         Compare two images with the same dimensions.
  maxv         Optimize the '-v' option.
  opti         Optimize the '-d' and '-v' options.
  info         View information for a blomp file.

Options:
  -d [int]           (--depth) Block depth.
  -v [float]     (--variation) Variation threshold.
  -o [string]       (--output) Output filename.
  -m [string]+     (--heatmap) Heatmap filename.
  -i [int]      (--iterations) Number of iterations.
  -c [string]     (--compfile) Comparison file.
  -x [target] [int] (--target) Target to reach.
  -g [string]+   (--genoutput) Regenerated image filename.
  -q                 (--quiet) Quiet. View less information.

Options with '+' have a default value when they are set to '+'.
For more details to their default values see their corresponding
help pages.

Supported image formats:
  Mode  | JPG PNG TGA BMP PSD GIF HDR PIC PNM
  ------+------------------------------------
  read  |  X   X   X   X   X   X   X   X   X
  write |  X   X   X   X
  For more details see https://github.com/nothings/stb
)";

// ---------- MODES ----------

static const char* encode =
R"(Help - Mode: 'enc'
Convert an image to a blomp file.
Available Options:
    -d, -v, -o, -m, -q

Input: Supported image file
Output: Blomp file

Defaults:
    -o      '${inFile%.*}.blp'
)";

static const char* decode =
R"(Help - Mode: 'dec'
Convert a blomp file to an image.
Available Options:
    -o, -m, -q

Input: Blomp file
Output: Supported image file

Defaults:
    -o      '${inFile%.*}.png'
)";

static const char* deencode =
R"(Help - Mode: 'denc'
Convert an image to blomp data and reconvert it back to an image.
Available Options:
    -d, -v, -o, -m, -g, -q

Input: Supported image file
Output: Supported image file
Side-data: Blomp file

Defaults:
    -o      '${inFile%.*}_DENC.png'
)";

static const char* compare =
R"(Help - Mode: 'comp'
Compare two images of any supported type and with the same dimensions.
Available Options:
    -c, -q

Input: Supported image file or blomp file
)";

static const char* maxvariation =
R"(Help - Mode: 'maxv'
Optimize the '-v' option to reach the given target.
Available Options:
    -d, -o, -m, -i, -x, -g, -q

Input: Supported image file or blomp file
Output: Blomp file
Side-data: Supported image file

Defaults:
    -o      '${inFile%.*}.blp'
    -g      '${inFile%.*}_MAXV.png'
)";

static const char* optimize =
R"(Help - Mode: 'opti'
Optimize the '-d' and '-v' options to reach the given target.
Available Options:
    -o, -m, -i, -x, -g, -q

Input: Supported image file or blomp file
Output: Blomp file
Side-data Supported image file.

Defaults:
    -o      '${inFile%.*}.blp'
    -g      '${inFile%.*}_OPTI.png'
)";

static const char* info =
R"(Help - Mode: 'info'
View information for a blomp file.
Available Options:
    -q

Input: Supported image file or blomp file
)";

// ---------- OPTIONS ----------

static const char* depth =
R"(Help - Option: '-d/--depth'
Description:
    Determines the block depth when generating a blomp block tree.
    Higher values may generate in 'blockier' images while possibly
    decreasing the file size.

Default: 4
Range: 0 - 10
)";

static const char* variation =
R"(Help - Option: '-v/--variation'
Description:
    Determines the variation threshold when generating a blomp block tree.
    Higher values may generate less detailed images while possibly
    decreasing the file size.

Default: 0.02
Range: 0.0 - 1.0
)";

static const char* output =
R"(Help - Option: '-o/--output'
Description:
    Name of the output file.

Default: Input filename with mode specific extension
)";

static const char* heatmap =
R"(Help - Option: '-m/--heatmap'
Description:
    Name of the heatmap file.
    When set to '+', '${inFile%.*}_HEAT.png' will be used.
    When not set, no heatmap file will be created.
    Must have the extension of a supported image format.
)";

static const char* iterations =
R"(Help - Option: '-i/--iterations'
Description:
    Number of iterations when determining the '-v' option.
    Higher values produce a more accurate approximation while
    increasing calculation time.
    When set to 0 the iteration stops after reaching a low target
    delta between the last two approximations.

Default: 4
Range: 0 - inf
)";

static const char* compfile =
R"(Help - Option: '-c/--compfile'
Description:
    Name of the comparision file.
)";

static const char* size =
R"(Help - Option: '-x/--target'
Description:
    Target for maxv calculations.

Target Values:
    size
    similarity

Range (size): 1 - inf
Range (similarity): 0 - 1000

Default: size [inputFileSize]
)";

static const char* genoutput =
R"(Help - Option: '-g/--genoutput'
Description:
    Name of the output file for side-generated data.
    When set to '+', a mode specific value will be used.
    When not set, no file containing side-generated data will be created.
)";

static const char* quiet =
R"(Help - Option: '-q/--quiet'
Description:
    Suppress non-important information.
)";

} // Namespace HelpText

namespace Blomp
{
    inline const char* getHelpText(const std::string& name)
    {
        if (name == "")
            return HelpText::general;
        if (name == "enc")
            return HelpText::encode;
        if (name == "dec")
            return HelpText::decode;
        if (name == "denc")
            return HelpText::deencode;
        if (name == "comp")
            return HelpText::compare;
        if (name == "maxv")
            return HelpText::maxvariation;
        if (name == "opti")
            return HelpText::optimize;
        if (name == "info")
            return HelpText::info;

        if (name == "-d" || name == "--depth")
            return HelpText::depth;
        if (name == "-v" || name == "--variation")
            return HelpText::variation;
        if (name == "-o" || name == "--output")
            return HelpText::output;
        if (name == "-m" || name == "--heatmap")
            return HelpText::heatmap;
        if (name == "-i" || name == "--iterations")
            return HelpText::iterations;
        if (name == "-c" || name == "--compfile")
            return HelpText::compfile;
        if (name == "-x" || name == "--target")
            return HelpText::size;
        if (name == "-g" || name == "--genoutput")
            return HelpText::genoutput;
        if (name == "-q" || name == "--quiet")
            return HelpText::quiet;

        return "No help page available.\n";
    }
}