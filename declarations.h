
#define MAX_PATH 260


struct DirectoryIndexQuad
{
	unsigned long offset;
	unsigned long length;
};
struct Quad
{
	char quad[4];
	unsigned long id;
	unsigned long fromProd;
	unsigned long toProd;
};

int FixFile();
int FixGGFR(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer);
int FixACTR(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer);
int FixGGAE(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer);
int FixTDT(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer);
bool VerifyQuad(char Quad[4],unsigned long ID,unsigned long *retQuadsArrNum);

