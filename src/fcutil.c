#include <stdio.h>
#include <string.h>
#include <fontconfig/fontconfig.h>

#include "fcutil.h"

int findMonospaceFont(char *buf) {
	if (!FcInit()) {
		fprintf(stderr, "Failed to initialize fontconfig\n");
		return 0;
	}
	FcConfig *conf = FcInitLoadConfigAndFonts();
	if (!conf) {
		fprintf(stderr, "Failed to create load fontconfig's fonts\n");
		return 0;
	}
	FcPattern *pat = FcNameParse((const FcChar8 *)"Mono");
	if (!pat) {
		fprintf(stderr, "Failed to find any fonts\n");
		return 0;
	}
	if (!FcConfigSubstitute(conf, pat, FcMatchPattern)) {
		fprintf(stderr, "Failed to substitute fontconfig pattern\n");
		return 0;
	}
	FcDefaultSubstitute(pat);
	FcResult result;
	FcPattern *font = FcFontMatch(conf, pat, &result);
	if (!font) {
		fprintf(stderr, "Failed to find font\n");
		return 0;
	}
	FcChar8 *file = NULL;
	if (FcPatternGetString(font, FC_FILE, 0, &file) != FcResultMatch) {
		fprintf(stderr, "Failed to find font match\n");
		return 0;
	}
	strcpy(buf, (char *)file);

	FcPatternDestroy(font);
	FcPatternDestroy(pat);
	FcConfigDestroy(conf);
	FcFini();

	return 1;
}
