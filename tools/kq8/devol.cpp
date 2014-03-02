#include <iostream>
#include <fstream>
#include <assert.h>
#include <string.h>
#include "tools/emi/filetools.h"

extern "C" {
#include "tools/kq8/blast.h"
}

using namespace std;

unsigned int g_blastSize;
unsigned char *g_blastBuf;

unsigned blast_read(void *how, unsigned char **buf) {
	*buf = g_blastBuf;
	return g_blastSize;
}

int blast_write(void *how, unsigned char *buf, unsigned len) {
	ofstream *ofs = reinterpret_cast<ofstream *>(how);
	ofs->write((char *)buf, len);
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " file.vol" <<endl;
		return 1;
	}

	ifstream fs(argv[1], fstream::in | fstream::binary);
	assert(fs.is_open());

	int currentPos = fs.tellg();

	char magic[8];
	fs.read(magic, 8);
	assert( 0 == memcmp(magic, "RESLIST", 8));

	currentPos = fs.tellg();
	while (true) {
		char filename[20];
		fs.read(filename, 20);
		if (fs.eof())
			break;
		currentPos = fs.tellg();
		int nextEntry = readInt(fs);
		currentPos = fs.tellg();
		int compressedSize = nextEntry - currentPos;
		cout << "File: " << filename << " (" << compressedSize << ")" << endl;

		ofstream ofs(filename);
		g_blastSize = compressedSize;
		g_blastBuf = new unsigned char[compressedSize];
		fs.read((char *)g_blastBuf, compressedSize);
		blast(&blast_read, NULL, &blast_write, &ofs);
		ofs.flush();
		ofs.close();
		delete[] g_blastBuf;
	}

	return 0;
}
