/*
 * inestrip - Strip iNES header from NES roms
 * Copyright (C) 2016 Timothy Redaelli
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _htobe32(num) (((num >> 24) & 0xff)	| \
			((num << 8) & 0xff0000)	| \
			((num >> 8) & 0xff00)	| \
			((num << 24) & 0xff000000))
#else
#define _htobe32(x) (x)
#endif

static void strip_rom(const char *path)
{
	uint8_t *buf;
	long fsize;
	uint32_t hdr = 0;
	FILE *fd = fopen(path, "rb");

	if (fd == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	if (fread(&hdr, 1, 4, fd) != 4) {
		perror("fread");
		exit(EXIT_FAILURE);
	}
	hdr = _htobe32(hdr);

	if (hdr != 0x4e45531a) {
		printf("Skipping: \"%s\"\n", path);
		fclose(fd);
		return;
	}

	printf("Stripping: \"%s\"\n", path);

	fseek(fd, 0L, SEEK_END);
	fsize = ftell(fd) - 16;
	fseek(fd, 16L, SEEK_SET);

	if (fsize < 0) {
		fputs("Wrong NES file (too small).", stderr);
		exit(EXIT_FAILURE);
	}

	buf = malloc(fsize);
	if (fread(buf, 1, fsize, fd) != (size_t) fsize) {
		perror("fread");
		exit(EXIT_FAILURE);
	}
	fclose(fd);

	if ((fd = fopen(path, "wb")) == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	fwrite(buf, 1, fsize, fd);
	fclose(fd);
	free(buf);
}

int main(int argc, char *argv[])
{
	DIR *pd;
	struct dirent *pdirent;
	const char *path = (argc > 1) ? argv[1] : ".";

	if ((pd = opendir(path)) == NULL) {
		perror("opendir");
		return EXIT_FAILURE;
	}

	while ((pdirent = readdir(pd)) != NULL) {
		if (!strcmp(pdirent->d_name, ".")
		    || !strcmp(pdirent->d_name, ".."))
			continue;
		if (!strncasecmp
		    (&pdirent->d_name[strlen(pdirent->d_name) - 4], ".nes", 4))
			strip_rom(pdirent->d_name);
	}

	closedir(pd);
	return EXIT_SUCCESS;
}
