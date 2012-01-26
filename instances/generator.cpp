#include <cstdio>
#include <cstdlib>

char *hhmmss(int minutes)
{
	char *ret = new char[9];
	int hh,mm;
	hh = minutes / 60;
	mm = minutes % 60;
	sprintf(ret, "%02d:%02d:00", hh, mm);
	return ret;
}

int main(int argc, char *argv[])
{
	// dwell, plat, open, close, maxL
    int no_platforms = 10;
    int no_dwells = 100;
	int open = 5*60;
	int total_opening = 16*60;
	int mean_lag = total_opening / no_dwells;
	int max_L = 5;

	sscanf(argv[1], "%d", &no_dwells);
	sscanf(argv[2], "%d", &no_platforms);
	sscanf(argv[3], "%d", &open);
	sscanf(argv[4], "%d", &total_opening);
	sscanf(argv[5], "%d", &max_L);

	printf("%d %d\n", no_dwells, no_platforms);
	char *zopen = hhmmss(open);
	char *zclose = hhmmss(open + total_opening);
	for (int k = 1; k <= no_platforms; k++)
		printf("%3d\t%s\t%s\n", k, zopen, zclose);
	delete []zopen;
	delete []zclose;

	int delta_max=5;
	int centro = open + delta_max;
	for (int i = 1; i <= no_dwells; i++) {
		int delta = rand() % delta_max + 1;
		int a = centro - delta;
		int b = centro + delta;
		char *za = hhmmss(a);
		char *zb = hhmmss(b);
		printf("%3d\t%s\t%s\t", i, za, zb);
		delete []za;
		delete []zb;
		int L = 0;
		for (int k = 1; k <= no_platforms && L < max_L; k++) {
			if (rand() % 2) {
				printf("%2d ", k);
				L++;
			}
		}
		printf("\n");
		if (rand() % 2)
			centro += rand() % (5 * delta_max);
		centro += rand() % delta_max;
	}
    printf("\n");
}

