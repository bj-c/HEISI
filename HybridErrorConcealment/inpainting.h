#include "image.h"

#define MAX(a, b)  (((a) > (b)) ? (a) : (b)) 
#define MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define PAINT_COLOR  RGB(0,255,0)
#define SOURCE 0
#define TARGET -1
#define BOUNDARY -2
#define winsize 3  
#define PI 3.14159265359

typedef struct
{
	double grad_x;
	double grad_y;
}gradient;

typedef struct
{
	double grad_x;
	double grad_y;
	double magnitude;
	bool check;
}gradcheck;

typedef struct
{
	double norm_x;
	double norm_y;
}norm;

enum PIXEL_POSITION
{
	PIXEL_LEFT ,
	PIXEL_RIGHT,
	PIXEL_TOP,
	PIXEL_BOTTOM,
	PIXEL_POSITION_COUNT,
};

enum PIXEL_RGB
{
	PIXEL_R,
	PIXEL_G,
	PIXEL_B,
	PIXEL_RGB_COUNT,
};

class inpainting
{
public:
	CImage * m_pImage;																				
	CImage * m_original;

	gradcheck check;
	bool m_bOpen;																					
	int m_width;																					
	int m_height;																					

	COLORREF * m_color;
	double * m_r;
	double * m_g;
	double * m_b;
	double * theta;
	double Interpolationthreshold;
	int m_top, m_bottom, m_left, m_right;															

	int * m_mark;																					
	double * m_confid;																				
	double * m_pri;																					
	double * m_gray;																				
	bool * m_source;																				
	double * m_threshold;
	double i_threshold;
	int Search_Range;
	int Tmin;		
	int Tmax;			
	
	inpainting(char * name, char * originalname);
	~inpainting(void);
	bool process(char * name, int searchrange, int Tmin, int Tmax);										
	void DrawBoundary(void);																			
	double ComputeConfidence(int i, int j);																
	double priority(int x, int y);																		
	double ComputeData(int i, int j);																	
	void Convert2Gray(void);																			
	gradient GetGradient(int i, int j);																	
	gradient GetColorGradient(int i, int j);															
	norm GetNorm(int i, int j);																			
	bool draw_source(void);																				
	bool PatchTexture(int x, int y,int &patch_x,int &patch_y, double *m_threshold);						
	bool update(int target_x, int target_y, int source_x, int source_y, double confid);					
	bool TargetExist(void);																				
	void UpdateBoundary(int i, int j);																	
	void UpdatePri(int i, int j);																		
	void filestruct(int width, int height, double *data);
// Threshold
	double ThresholdDecision(int x, int y);
	void InterpolationThreshold();
// grad
	gradcheck GradCheck(int x, int y);	
// Interpolation
	bool updateForInterpolation(int target_x, int target_y, double* LEFT, double* RIGHT);			
	void Interpolation(int x,int y, double* pixel,int mode, double* LEFT, double* RIGHT);
	void NeighborInterpolation(int x, int y, double *temp);
	void HorizontInterpolation(int x, int y, double *temp);
	void VerticalInterpolation(int x, int y, double *temp);
	void OrientInterpolation(int x, int y, double *temp, int mode, double* LEFT, double* RIGHT);
	void Paint(int x, int y, double *temp);
	void magnitude(int x, int y);

	DWORD getProcessTime();
	double getPSNR();
	BOOL SaveResultImage();
	BOOL SaveResultValue(const char* file_path);
private:
	char m_InputFileName[MAX_PATH];
	DWORD m_process_time;
	double m_PSNR;
};