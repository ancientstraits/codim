#include <stdio.h>
#include <string.h>
#include <fontconfig/fontconfig.h>

#include "fcutil.h"

char* findMonospaceFont() {
    if (!FcInit()) {
        fprintf(stderr, "Failed to initialize fontconfig\n");
        return NULL;
    }
    FcConfig* conf = FcInitLoadConfigAndFonts();
    if (!conf) {
        fprintf(stderr, "Failed to create load fontconfig's fonts\n");
        return NULL;
    }
	FcPattern *pat = FcNameParse((const FcChar8 *)"Mono");
    if (!pat) {
        fprintf(stderr, "Failed to find any fonts\n");
        return NULL;
    }
	if (!FcConfigSubstitute(conf, pat, FcMatchPattern)) {
        fprintf(stderr, "Failed to substitute fontconfig pattern\n");
        return NULL;
    }
	FcDefaultSubstitute(pat);
	char *fontFile;
	FcResult result;
	FcPattern *font = FcFontMatch(conf, pat, &result);
	if (!font) {
        fprintf(stderr, "Failed to find font\n");
        return NULL;
    }
    FcChar8 *file = NULL;
    if (FcPatternGetString(font, FC_FILE, 0, &file) != FcResultMatch) {
        fprintf(stderr, "Failed to find font match\n");
        return NULL;
    }
    fontFile = strdup((char *)file);
    FcPatternDestroy(font);
	FcPatternDestroy(pat);
    FcConfigDestroy(conf);
    FcFini();

    return fontFile;
}
