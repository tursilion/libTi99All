// String code for the TI-99/4A by Tursi
// You can copy this file and use it at will ;)

int stricmp(const char *s1, const char *s2) {
	while (*s1) {
        unsigned char a, b;
		if (*s2 == '\0') {
			return 1;
		}
        // this is a little imperfect, some punctuation may match, but it's very unlikely
        a=*s1;
        if (a > 'Z') a-=32;
        b=*s2;
        if (b > 'Z') b-=32;
		if (a != b) {
			return *s1 - *s2;
		}
		++s1;
		++s2;
	}
	if (*s2 != '\0') {
		return -1;
	}
	return 0;
}
