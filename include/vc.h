//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Bin�rio/Cinzentos=1; RGB=3
	int levels;				// Bin�rio=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perímetro
	int label;					// Etiqueta
	int color;
	int sign;
} OVC;


enum signs { STOP, CAR, HIGHWAY, ARROWLEFT, ARROWRIGHT, FORBIDDEN } ;
enum color { RED, GREEN, BLUE } ;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

int vc_scale_gray_to_rgb(IVC *src, IVC *dst);
int vc_gray_histogram_show(IVC *src, IVC *dst);
int vc_gray_histogram_equalization(IVC *src, IVC *dst);
int vc_gray_highpass_filter(IVC *src, IVC *dst);
int vc_gray_highpass_filter_enhance(IVC *src, IVC *dst, int gain);
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);
int vc_gray_lowpass_gaussian_filter(IVC *src, IVC *dst);

int vc_rgb_to_hsv(IVC *src, int tipo);
int vc_gray_edge_sobel(IVC *src, IVC *dst, float th);

int vc_rgb_to_gray(IVC *src, IVC *dst);
int vc_gray_negative(IVC *srcdst);
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold);

int vc_binary_open(IVC *src, IVC *dst, int kernel);
int vc_binary_close(IVC *src, IVC *dst, int kernel);
int vc_binary_dilate(IVC *src, IVC *dst, int kernel);
int vc_binary_erode(IVC *src, IVC *dst, int kernel);

void vc_sobel_edge_detection(IVC* image);

int convertToColorImage(IVC* grayImage,IVC* colorImage);
void drawBoundingBox(IVC* image, int x, int y, int width, int height);

OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels,int color);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);

int drawBox(IVC* img, int x, int y, int width, int height) ;
int makeBlack(IVC* img) ;

int vc_show_rb_objects(IVC* image);
int vc_show_red_objects(IVC* image);
int vc_show_blue_objects(IVC* image);
int detect_sign(OVC*blobs,int blobId,IVC *tempGray);
