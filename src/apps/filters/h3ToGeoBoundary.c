/*
 * Copyright 2016-2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file
 * @brief stdin/stdout filter that converts from integer H3 indexes to lat/lon
 * cell boundaries
 *
 *  usage: `h3ToGeoBoundary [outputMode kmlName kmlDesc]`
 *
 *  The program reads H3 indexes from stdin and outputs the corresponding
 *  cell boundaries to stdout, until EOF is encountered.
 *
 *  `outputMode` indicates the type of output; the choices are 0 for
 *       plain text output (the default) and 1 for KML output
 *
 *  `kmlName` indicates the string for the name tag in KML output (only used
 *       when `outputMode` == 1). The default is "geo from H3".
 *
 *  `kmlName` indicates the string for the desc tag in KML output (only used
 *       when `outputMode` == 1). The default is "generated by h3ToGeoBoundary".
 *
 *  Examples:
 *
 *     `h3ToGeoBoundary < indexes.txt`
 *        - outputs plain text cell boundaries for the H3 indexes contained
 *          in the file `indexes.txt`
 *
 *     `h3ToGeoBoundary 1 "kml file" "h3 cells" < indexes.txt > cells.kml`
 *        - creates the KML file `cells.kml` containing the cell boundaries for
 *         all of the H3 indexes contained in the file `indexes.txt`.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "baseCells.h"
#include "coordijk.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "h3IndexFat.h"
#include "h3api.h"
#include "kml.h"
#include "utility.h"
#include "vec2d.h"

void doCell(H3Index h, int isKmlOut) {
    GeoBoundary b;
    H3_EXPORT(h3ToGeoBoundary)(h, &b);

    char label[BUFF_SIZE];
    H3_EXPORT(h3ToString)(h, label, BUFF_SIZE);

    if (isKmlOut) {
        outputBoundaryKML(&b, label);
    } else {
        printf("%s\n", label);
        geoBoundaryPrintln(&b);
    }
}

int main(int argc, char *argv[]) {
    // check command line args
    if (argc > 5) {
        fprintf(stderr, "usage: %s [outputMode kmlName kmlDesc]\n", argv[0]);
        exit(1);
    }

    int isKmlOut = 0;
    if (argc > 1) {
        if (!sscanf(argv[1], "%d", &isKmlOut))
            error("outputMode must be an integer");

        if (isKmlOut != 0 && isKmlOut != 1) error("outputMode must be 0 or 1");
        char *defaultKmlName = "geo from H3";
        char *defaultKmlDesc = "from h3ToGeo";

        char *kmlName = defaultKmlName;
        char *kmlDesc = defaultKmlDesc;

        if (argc > 2) {
            kmlName = argv[2];
            if (argc > 3) kmlDesc = argv[3];
        }

        kmlPtsHeader(kmlName, kmlDesc);
    }

    // process the indexes on stdin
    char buff[BUFF_SIZE];
    while (1) {
        // get an index from stdin
        if (!fgets(buff, BUFF_SIZE, stdin)) {
            if (feof(stdin))
                break;
            else
                error("reading H3 index from stdin");
        }

        H3Index h3 = H3_EXPORT(stringToH3)(buff);
        doCell(h3, isKmlOut);
    }

    if (isKmlOut) kmlPtsFooter();
}
