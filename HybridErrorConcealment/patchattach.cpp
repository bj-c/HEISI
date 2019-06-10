#include "stdafx.h"

inpainting::inpainting(char * inputname, char * originalname)
{
	m_process_time = 0;
	COLORREF temp;
	m_pImage = new CImage;
	m_original=new CImage;

	if(!m_pImage->Load((LPCTSTR)inputname))
		m_bOpen=false;
	if( m_original->Load((LPCTSTR)originalname) ) 

	if(m_bOpen)
	{	
		m_pImage->Lock();	
		m_width = m_pImage->m_BMPwidth;
		m_height = m_pImage->m_BMPheigth;
		m_mark = new int[m_width*m_height];
		m_confid = new double[m_width*m_height];
		memset( m_confid, 0, m_width*m_height*sizeof(double) );
		m_pri = new double[m_width*m_height];
		memset(m_pri, 0, m_width*m_height*sizeof(double));
		m_gray  = new double[m_width*m_height];
		m_source = new bool[m_width*m_height];
		m_color = new COLORREF[m_width*m_height];
		m_r = new double[m_width*m_height];
		m_g = new double[m_width*m_height];
		m_b = new double[m_width*m_height];
		theta=new double[m_width*m_height];
		m_threshold=new double[m_width*m_height];
		memset(m_threshold, 0, m_width*m_height*sizeof(double));
	}
	else printf("one or more file is not opened!\n");
	m_top = m_height;  // initialize the rectangle area
	m_bottom = 0; 
	m_left = m_width;
	m_right = 0;
	m_PSNR=0.0f;

	for(int y = 0; y<m_height; y++)
		for(int x = 0; x<m_width; x++)
		{
			m_pImage->GetPixel(x,y,temp);
			m_color[y*m_width+x] = temp;
			m_r[y*m_width+x] = GetRValue(temp);
			m_g[y*m_width+x] = GetGValue(temp);
			m_b[y*m_width+x] = GetBValue(temp);
		}
}
inpainting::~inpainting(void)
{
	if(m_bOpen)
	{
		if(m_pImage)delete m_pImage;
		if(m_mark)delete m_mark;
		if(m_source)delete m_source;
		if(m_color)delete m_color;
		if(m_confid)delete m_confid;
		if(m_gray)delete m_gray;
		if(m_pri)delete m_pri;
		if(m_r)delete m_r;
		if(m_g)delete m_g;
		if(m_b)delete m_b;
		if(m_original)delete m_original;
		if(theta)delete theta;
		if(m_threshold)delete m_threshold;
	}
}
void inpainting::Convert2Gray(void)
{
	COLORREF  cc;
	double r,g,b;
	for(int y = 0; y<m_height; y++)
		for(int x = 0; x<m_width; x++)
		{
			m_pImage->GetPixel(x,y,cc);
			r = GetRValue(cc);
			g = GetGValue(cc); 
			b = GetBValue(cc);
			m_gray[y*m_width+x] = (double)((r*3735 + g*19267 + b*9765)/32767);
		}
}
void inpainting::DrawBoundary(void)
{
	COLORREF color;	
	for(int y = 0; y<m_height; y++)
		for(int x = 0; x<m_width; x++)
		{
			m_pImage->GetPixel(x,y,color);   
			if(color==PAINT_COLOR)
			{
				m_mark[y*m_width+x] = TARGET;
				m_confid[y*m_width+x] = 0;
			}
			else {
				m_mark[y*m_width+x] = SOURCE;
				m_confid[y*m_width+x] = 1;
			}
		}
		for(int j= 0; j< m_height; j++)
			for(int i = 0; i< m_width; i++)
			{
				if(m_mark[j*m_width+i]==TARGET)
				{
					if(i<m_left)m_left = i;
					if(i>m_right)m_right = i;
					if(j>m_bottom)m_bottom = j;
					if(j<m_top)m_top = j;
					if(j==m_height-1 || j==0 || i==0 || i==m_width-1 || m_mark[(j-1)*m_width+i] == SOURCE || m_mark[j*m_width+i-1] == SOURCE || m_mark[j*m_width+i+1] == SOURCE || m_mark[(j+1)*m_width+i] == SOURCE)
						m_mark[j*m_width+i] = BOUNDARY;
				}
			}
}
double inpainting::priority(int i, int j)
{
	double confidence, data;
	confidence = ComputeConfidence(i,j);
	data = ComputeData(i,j);
	return confidence*data;
}

double inpainting::ComputeConfidence(int i, int j) 
{
	double confidence=0;
	for(int y = MAX(j-winsize,0); y<= MIN(j+winsize,m_height-1); y++)
		for(int x = MAX(i-winsize,0); x<=MIN(i+winsize, m_width-1); x++)
			confidence+= m_confid[y*m_width+x];
	confidence/= (winsize*2+1)*(winsize*2+1);
	return confidence;
}

double inpainting::ComputeData(int i, int j)
{
	gradient grad, temp, grad_T;
	grad.grad_x=0;
	grad.grad_y=0;
	double result;
	double magnitude;
	double magnitude_buffer=0;
	int buffer_count=0;
	double max=0;
	int x, y;
	for(y = MAX(j -winsize,0); y<= MIN(j+winsize,m_height-1); y++)
		for( x = MAX(i-winsize,0); x<=MIN(i+winsize, m_width-1); x++)
		{
			if(m_mark[y*m_width+x] >=0)
			{
				if(m_mark[y*m_width+x+1]<0 || m_mark[y*m_width+x-1]<0 || m_mark[(y+1)*m_width+x]<0 || m_mark[(y-1)*m_width+x]<0)continue;

				temp = GetGradient(x,y); 
				magnitude = temp.grad_x * temp.grad_x + temp.grad_y * temp.grad_y;
				magnitude_buffer+=magnitude;
				buffer_count++;

				if(magnitude>max)
				{
					grad.grad_x = temp.grad_x;
					grad.grad_y = temp.grad_y;
					max = magnitude;
				}
			}
		}
		magnitude_buffer/=buffer_count;

		m_threshold[j*m_width+i]=magnitude_buffer;

		grad_T.grad_x = grad.grad_y;
		grad_T.grad_y = -grad.grad_x;

		norm nn = GetNorm(i,j);
		result = nn.norm_x*grad_T.grad_x+nn.norm_y*grad_T.grad_y;
		result/=255;
		result = fabs(result);			
		return result;
}
gradient inpainting::GetGradient(int i, int j)
{
	gradient result;
	result.grad_x = (m_gray[j*m_width+i+1] - m_gray[j*m_width+i-1])/2.0;
	result.grad_y = (m_gray[(j+1)*m_width +i] - m_gray[(j-1)*m_width+i])/2.0;
	if(i==0)
		result.grad_x = m_gray[j*m_width+i+1] - m_gray[j*m_width+i];

	if(i==m_width-1)
		result.grad_x = m_gray[j*m_width+i] - m_gray[j*m_width+i-1];

	if(j==0)
		result.grad_y = m_gray[(j+1)*m_width +i] - m_gray[j*m_width+i];

	if(j==m_height-1)
		result.grad_y = m_gray[j*m_width +i] - m_gray[(j-1)*m_width+i];

	return result;
}
norm inpainting::GetNorm(int i, int j)
{
	norm result;
	int num=0;
	int neighbor_x[9];
	int neighbor_y[9];
	int record[9];
	int count = 0;
	for(int y = MAX(j-1,0); y<=MIN(j+1,m_height-1); y++)
		for(int x = MAX(i-1,0); x<=MIN(i+1,m_width-1); x++)
		{
			count++;
			if(x==i&&y==j)continue;
			if(m_mark[y*m_width+x]==BOUNDARY)
			{
				num++;
				neighbor_x[num] = x;
				neighbor_y[num] = y;		
				record[num]=count;
			}
		}
		if(num==0||num==1) 
		{
			result.norm_x = 0.6;
			result.norm_y = 0.8;
			return result;
		}
		int n_x = neighbor_x[2]-neighbor_x[1];
		int n_y = neighbor_y[2]-neighbor_y[1];
		int temp=n_x;
		n_x = n_y;
		n_y = temp;
		double square = pow(double(n_x*n_x + n_y*n_y),0.5);

		result.norm_x = n_x/square;
		result.norm_y =n_y/square;
		return result;
}

bool inpainting::draw_source(void) 
{
	bool flag;
	for(int j = 0; j<m_height; j++)
		for(int i = 0; i<m_width; i++)
		{
			flag=1;

			if( i<winsize || j<winsize || i>=m_width-winsize || j>=m_height-winsize) 
				m_source[j*m_width+i] = false;
			else
			{
				for(int y = j-winsize; y<=j+winsize; y++)
				{
					for(int x = i-winsize; x<=i+winsize; x++)
					{
						if(m_mark[y*m_width+x]!=SOURCE)
						{
							m_source[j*m_width+i]=false;
							flag = false;
							break;			
						}		
					}
					if(flag==false)break;
				}
				if(flag!=false)m_source[j*m_width+i]=true;
			}
		}
		return true;
}
bool inpainting::PatchTexture(int x, int y, int &patch_x, int &patch_y, double *m_threshold)
{
	double temp_r;
	double temp_g;
	double temp_b;
	double min=INT_MAX; 
	int source_x, source_y, target_x, target_y;
	double SMD,MSE,total_sum;
	double side_count, count;
	int size=0;

	double threshold=ThresholdDecision(x, y);

	for(int j =  MAX(y - Search_Range, 0); j<MIN(y + Search_Range, m_height); j++)
		for(int i = MAX(x - Search_Range, 0) ; i<MIN(x + Search_Range, m_width); i++){
			if(m_source[j*m_width+i]==false)continue;
			MSE=0;
			count=0;
			SMD=0;
			side_count=0;


			for(int iter_y=(-1)*winsize; iter_y<=winsize; iter_y++){
				for(int iter_x=(-1)*winsize; iter_x<=winsize; iter_x++){
					source_x = i+iter_x;
					source_y = j+iter_y;

					target_x = x+iter_x;
					target_y = y+iter_y;

					if(target_x<0||target_x>=m_width||target_y<0||target_y>=m_height)continue;
					if(source_x<0||source_x>=m_width||source_y<0||source_y>=m_height)continue;

					if(m_mark[target_y*m_width+target_x]>=0)
					{
						temp_r = m_r[target_y*m_width+target_x]-m_r[source_y*m_width+source_x];
						temp_g = m_g[target_y*m_width+target_x]-m_g[source_y*m_width+source_x];
						temp_b = m_b[target_y*m_width+target_x]-m_b[source_y*m_width+source_x];
						MSE+= temp_r*temp_r + temp_g*temp_g + temp_b*temp_b;
						count++;
						if(m_mark[(target_y+1)*m_width+target_x]==-2 && source_y+1<m_height)
						{
							temp_r = m_r[target_y*m_width+target_x]-m_r[(source_y+1)*m_width+source_x];
							temp_g = m_g[target_y*m_width+target_x]-m_g[(source_y+1)*m_width+source_x];
							temp_b = m_b[target_y*m_width+target_x]-m_b[(source_y+1)*m_width+source_x];
							SMD+= temp_r*temp_r + temp_g*temp_g + temp_b*temp_b;
							side_count++;
						}
						if(m_mark[(target_y-1)*m_width+target_x]==-2 && source_y-1 >= 0)
						{
							temp_r = m_r[target_y*m_width+target_x]-m_r[(source_y-1)*m_width+source_x];
							temp_g = m_g[target_y*m_width+target_x]-m_g[(source_y-1)*m_width+source_x];
							temp_b = m_b[target_y*m_width+target_x]-m_b[(source_y-1)*m_width+source_x];
							SMD+= temp_r*temp_r + temp_g*temp_g + temp_b*temp_b;
							side_count++;
						}
						if(m_mark[target_y*m_width+(target_x+1)]==-2 && source_x+1 <m_width )
						{
							temp_r = m_r[target_y*m_width+target_x]-m_r[source_y*m_width+(source_x+1)];
							temp_g = m_g[target_y*m_width+target_x]-m_g[source_y*m_width+(source_x+1)];
							temp_b = m_b[target_y*m_width+target_x]-m_b[source_y*m_width+(source_x+1)];
							SMD+= temp_r*temp_r + temp_g*temp_g + temp_b*temp_b;
							side_count++;
						}
						if(m_mark[target_y*m_width+(target_x-1)]==-2 && source_x-1 >= 0){
							temp_r = m_r[target_y*m_width+target_x]-m_r[source_y*m_width+(source_x-1)];
							temp_g = m_g[target_y*m_width+target_x]-m_g[source_y*m_width+(source_x-1)];
							temp_b = m_b[target_y*m_width+target_x]-m_b[source_y*m_width+(source_x-1)];
							SMD+= temp_r*temp_r + temp_g*temp_g + temp_b*temp_b;
							side_count++;
						}
					}
				}
			}
			MSE/=count;
			SMD/=side_count;
			double weight=1;
			weight= (count / (count + side_count));
			total_sum=weight * MSE + (1-weight) * SMD;

			if(total_sum<min )
			{
				min=total_sum;
				patch_x = i;
				patch_y = j;
			}
		}
		if( min > threshold )
		{
			return false;
		}

		return true;
}
bool inpainting::update(int target_x, int target_y, int source_x, int source_y, double confid)
{
	int x0,y0,x1,y1;
	for(int iter_y=(-1)*winsize; iter_y<=winsize; iter_y++)
		for(int iter_x=(-1)*winsize; iter_x<=winsize; iter_x++)
		{
			x0 = source_x+iter_x;
			y0 = source_y+iter_y;

			x1 = target_x + iter_x;
			y1 = target_y + iter_y;
			if(m_mark[y1*m_width+x1]<0)
			{
				m_pImage->SetPixel(x1,y1,m_color[y0*m_width+x0]);
				m_color[y1*m_width+x1] = m_color[y0*m_width+x0];
				m_r[y1*m_width+x1] = m_r[y0*m_width+x0];
				m_g[y1*m_width+x1] = m_g[y0*m_width+x0];
				m_b[y1*m_width+x1] = m_b[y0*m_width+x0];
				m_gray[y1*m_width+x1] = (double)((m_r[y0*m_width+x0]*3735 + m_g[y0*m_width+x0]*19267 + m_b[y0*m_width+x0]*9765)/32767);
				m_confid[y1*m_width+x1] = confid;
			}
		}
		return true;
}

bool inpainting::TargetExist(void)
{
	for(int j= m_top; j<=m_bottom; j++)
		for(int i = m_left; i<= m_right; i++)
			if(m_mark[j*m_width+i]<0)
				return true;

	return false;
}

void inpainting::UpdateBoundary(int i, int j)
{
	int x, y;
	COLORREF color;	

	for(y = MAX(j -winsize-2,0); y<= MIN(j+winsize+2,m_height-1); y++)
		for( x = MAX(i-winsize-2,0); x<=MIN(i+winsize+2, m_width-1); x++)
		{
			m_pImage->GetPixel(x,y,color);
			if(color==PAINT_COLOR)m_mark[y*m_width+x] = TARGET;
			else m_mark[y*m_width+x] = SOURCE;
		}

		for(y = MAX(j -winsize-2,0); y<= MIN(j+winsize+2,m_height-1); y++)
			for( x = MAX(i-winsize-2,0); x<=MIN(i+winsize+2, m_width-1); x++)
			{
				if(m_mark[y*m_width+x]==TARGET)
				{
					if(y==m_height-1||y==0||x==0||x==m_width-1||m_mark[(y-1)*m_width+x]==SOURCE||m_mark[y*m_width+x-1]==SOURCE
						||m_mark[y*m_width+x+1]==SOURCE||m_mark[(y+1)*m_width+x]==SOURCE)m_mark[y*m_width+x] = BOUNDARY;
				}
			}		
}
void inpainting::UpdatePri(int i, int j)
{
	int x,y;
	for(y = MAX(j -winsize-3,0); y<= MIN(j+winsize+3,m_height-1); y++)
		for( x = MAX(i-winsize-3,0); x<=MIN(i+winsize+3, m_width-1); x++)if(m_mark[y*m_width+x] == BOUNDARY)
			m_pri[y*m_width+x] = priority(x,y);
}