/*
 *  FileHandle.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 04/20/12.
 *  GPL
 *
 */

struct FileHandle {
	FILE *fp;
	unsigned int off;
	unsigned int len;
  const char *name;
};

enum {
  MODELS,
  SOUNDS,
  TEXTURES,
  LEVELS
};
