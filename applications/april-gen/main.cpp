#include <stdio.h>
#include <apriltags/apriltag.h>
#include <apriltags/tag36h11.h>

void write_tags( FILE* f, int x, int y, unsigned char* tag, int id, float scale = 25.4, float wb = 0.0f )
{
  // Convert scale to EPS units, default set to 1"
  scale *= 72;
  scale /= 25.4;
  scale /= 8;

  fprintf(f, "%d %d translate\n", x, y);
  for(int ii = 0; ii < 8; ii++) {
    for(int jj = 0; jj < 8; jj++){
      if ((ii == 0) || (ii == 7) || (jj == 0) || (jj == 7)) {
        fprintf(f, "%f %f %f %f rectfill\n", ii*scale, jj*scale, scale, scale);
      }
      else{
        int i = ii - 1;
        int j = jj - 1;
        if (tag[j + 6*i] == 0){
          fprintf(f, "%f %f %f %f rectfill\n", ii*scale, jj*scale, scale, scale);
        }
      }
    }
  }
  if (wb != 0.0f) {
    // Convert from mm to EPS units [ 72eps = 1in = 25.4mm ]
    wb *= 72;
    wb /= 25.4;
    fprintf(f, "%f %f moveto\n", -wb, -wb);
    fprintf(f, "%f %f lineto\n", wb + 8*scale, -wb);
    fprintf(f, "%f %f lineto\n", wb + 8*scale, wb + 8*scale);
    fprintf(f, "%f %f lineto\n", -wb, wb + 8*scale);
    fprintf(f, "%f %f lineto\n", -wb, -wb);
    fprintf(f, "closepath stroke\n", -wb, -wb);
  }
  fprintf(f, "%f %f translate\n", wb + 10*scale, scale);
  fprintf(f, "90 rotate\n");
  fprintf(f, "newpath\n0 0 moveto\n");
  fprintf(f, "(Tag: %d) show\n", id);
  fprintf(f, "-90 rotate\n");
  fprintf(f, "%f %f translate\n", -(wb + 10*scale), -scale);
  fprintf(f, "%d %d translate\n", -x, -y);
}

void tag_from_id( int id, unsigned char* tag)
{
  april_tag_family_t* tf = tag36h11_create();

  unsigned long value = tf->codes[id];
  memset(tag, 0, 36);

  bool t_reverse[36];
  int idx = 0;
  for (int count = 0; count < 9; count++) {
    unsigned long long temp = value - ((value >> 4) << 4);
    t_reverse[idx + 0] = temp & 1;
    t_reverse[idx + 1] = temp & 2;
    t_reverse[idx + 2] = temp & 4;
    t_reverse[idx + 3] = temp & 8;
    idx += 4;
    value >>= 4;
  }
  for (int count = 0; count < 36; count++){
    tag[count] = t_reverse[35 - count];
  }
  tag36h11_destroy(tf);
}

void write_csv( FILE* f, int x, int y, int id )
{
  id *= 100;
  float del = (80 / 72.0f) * ( 0.0254);
  float xf = 0.0254*(x / 72.0f);
  float yf = 0.0254*(y / 72.0f);
  fprintf(f, "%d, %f, %f,  0\n", id, xf, yf);
  fprintf(f, "%d, %f, %f,  0\n", id + 1, xf + del, yf);
  fprintf(f, "%d, %f, %f,  0\n", id + 2, xf + del, yf + del);
  fprintf(f, "%d, %f, %f,  0\n", id + 3, xf, yf + del);
}

int main( int argc, char** argv )
{  
  unsigned char* tag = new unsigned char[36];

  if ((argc % 3 != 0) || (argc < 5)) {
    fprintf(stdout, "Usage: %s scale wb tag1 x1 y1 tag2 x2 y2 . . . \n", argv[0]);
    fprintf(stdout, "Scale and wb (white border) are in units of mm\n");
    fprintf(stdout, "Assigning a value of 0 to 'wb' will not draw an outline.\n");
    exit(0);
  }

  FILE* f = fopen("APRIL_tags.eps", "w");
  FILE* csv = fopen("Data.csv", "w");
  fprintf(f, "%%!PS-Adobe EPSF-3.0\n");
  fprintf(f, "%%%%Boundingbox: 0 0 612 792\n");
  fprintf(f, "/Times-Roman findfont 20 scalefont setfont\n");

  int count = 3;
  float scale = atof(argv[1]);
  float wb = atof(argv[2]);
  while (count < argc){
    tag_from_id(atoi(argv[count]), tag);
    write_tags(f, atoi(argv[count + 1]), atoi(argv[count + 2]),
        tag, atoi(argv[count]), scale, wb);
    write_csv(csv, atoi(argv[count + 1]), atoi(argv[count + 2]),
        atoi(argv[count]));
    count += 3;
  }

  fprintf(f, "showpage\n");
  fclose(f);
  fclose(csv);
  delete[] tag;
  return 0;
}
