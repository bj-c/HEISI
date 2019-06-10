#include "stdafx.h"

bool inpainting::updateForInterpolation(int target_x, int target_y, double* LEFT, double* RIGHT)
{
	gradcheck G;

	double temp[PIXEL_RGB_COUNT]={0};
	bool boundary_check=false;
	bool mode[10]={false};
	double theta;

	G=GradCheck(target_x, target_y);

	if(G.check)
	{
		if(G.magnitude>i_threshold)
		{
			theta=atan(G.grad_x/G.grad_y);
			if(theta<0)
				theta+=PI;

			for(int i=0; i<9; i++)
			{
				if(LEFT[i] <= theta && theta < RIGHT[i])
				{
					mode[i]=true;
					boundary_check=true;
					break;
				}
			}
		}
	}
	if(boundary_check==false)
		mode[9]=true;

	for(int i=0; i<=9; i++)
		if(mode[i])
		{
			Interpolation(target_x,target_y, temp, i, LEFT, RIGHT);
			break;
		}
		Paint(target_x, target_y, temp);
		return true;
}

void inpainting::Interpolation(int target_x,int target_y, double* temp, int mode, double* LEFT, double* RIGHT)
{
	if(mode == 9)
	{
		NeighborInterpolation(target_x,target_y,temp);
	}
	else if(mode ==4) 
	{
		VerticalInterpolation(target_x,target_y,temp);
	}
	else
	{
		OrientInterpolation(target_x, target_y, temp, mode, LEFT,  RIGHT);
	}

	for( int i = 0; i < 3; i++ )
		if( temp[i] > 255 )
			temp[i] = 255;
		else if( temp[i] < 0 )
			temp[i] = 0;

}

gradient inpainting::GetColorGradient(int i, int j)
{
	gradient result;
	result.grad_x=0;
	result.grad_y=0;

	result.grad_x += (m_r[j*m_width+i+1] - m_r[j*m_width+i-1])/2.0;
	result.grad_x += (m_g[j*m_width+i+1] - m_g[j*m_width+i-1])/2.0;
	result.grad_x += (m_b[j*m_width+i+1] - m_b[j*m_width+i-1])/2.0;

	result.grad_y += (m_r[(j+1)*m_width +i] - m_r[(j-1)*m_width+i])/2.0;
	result.grad_y += (m_g[(j+1)*m_width +i] - m_g[(j-1)*m_width+i])/2.0;
	result.grad_y += (m_b[(j+1)*m_width +i] - m_b[(j-1)*m_width+i])/2.0;

	if(i==0)
	{
		result.grad_x=0;
		result.grad_x =+ m_gray[j*m_width+i+1] - m_gray[j*m_width+i];
		result.grad_x =+ m_gray[j*m_width+i+1] - m_gray[j*m_width+i];
		result.grad_x =+ m_gray[j*m_width+i+1] - m_gray[j*m_width+i];
	}

	if(i==m_width-1)
	{
		result.grad_x=0;
		result.grad_x =+ m_r[j*m_width+i] - m_r[j*m_width+i-1];
		result.grad_x =+ m_g[j*m_width+i] - m_g[j*m_width+i-1];
		result.grad_x =+ m_b[j*m_width+i] - m_b[j*m_width+i-1];
	}

	if(j==0)
	{
		result.grad_y=0;
		result.grad_y =+ m_r[(j+1)*m_width +i] - m_r[j*m_width+i];
		result.grad_y =+ m_g[(j+1)*m_width +i] - m_g[j*m_width+i];
		result.grad_y =+ m_b[(j+1)*m_width +i] - m_b[j*m_width+i];
	}

	if(j==m_height-1)
	{
		result.grad_y=0;
		result.grad_y =+ m_r[j*m_width +i] - m_r[(j-1)*m_width+i];
		result.grad_y =+ m_g[j*m_width +i] - m_g[(j-1)*m_width+i];
		result.grad_y =+ m_b[j*m_width +i] - m_b[(j-1)*m_width+i];
	}
	result.grad_x/=3;
	result.grad_y/=3;
	return result;
}
gradcheck inpainting::GradCheck(int x, int y)
{
	check.check=false;
	check.grad_x=0;
	check.grad_y=0;
	check.magnitude=0;
	for(int j=-2; j<=2; j++)
		for(int i=-2; i<=2; i++)
			magnitude(i+x,j+y );

	return check;
}
double inpainting::ThresholdDecision(int x, int y)
{
	double threshold=m_threshold[y*m_width+x];;

	if(Tmax<threshold)
		threshold=Tmax;
	else if(Tmin>threshold)
		threshold=Tmin;

	return threshold;
}
void inpainting::Paint(int x, int y, double *temp)
{
		m_r[x + m_width * y] = temp[0];
		m_g[x + m_width * y] = temp[1];
		m_b[x + m_width * y] = temp[2];

		m_gray[y*m_width+x] = (double)(temp[0]*3735 + temp[1] *19267 + temp[2]) / 32767;
		m_pImage->SetPixel(x,y,RGB(temp[0],temp[1],temp[2]));
		m_confid[y*m_width+x] = ComputeConfidence(x, y) ;
		m_pri[y*m_width+x] = 0;
}
void inpainting::NeighborInterpolation(int x, int y, double *temp)
{
	int wight=0;
	if( m_mark[y*m_width+(x+1)] == SOURCE )
	{
		temp[0] += m_r[y*m_width+(x+1)];
		temp[1] += m_g[y*m_width+(x+1)];
		temp[2] += m_b[y*m_width+(x+1)];
		wight++;
	}
	if( m_mark[y*m_width+(x-1)] == SOURCE )
	{
		temp[0] += m_r[y*m_width+(x-1)];
		temp[1] += m_g[y*m_width+(x-1)];
		temp[2] += m_b[y*m_width+(x-1)];
		wight++;
	}
	if( m_mark[(y+1)*m_width+x] == SOURCE )
	{
		temp[0] += m_r[(y+1)*m_width+x];
		temp[1] += m_g[(y+1)*m_width+x];
		temp[2] += m_b[(y+1)*m_width+x];
		wight++;
	}
	if( m_mark[(y-1)*m_width+x] == SOURCE )
	{
		temp[0] += m_r[(y-1)*m_width+x];
		temp[1] += m_g[(y-1)*m_width+x];
		temp[2] += m_b[(y-1)*m_width+x];
		wight++;
	}
	for( int i = 0; i < 3; i++ )
		temp[i] /= wight;
}
void inpainting::OrientInterpolation(int x, int y, double *temp, int mode, double* LEFT, double* RIGHT)
{
	double max=0;
	int sign_x[2], sign_y[2];
	int pixel_x[2]={x-1,x+1};
	int pixel_y[2]={y, y};
	double weight[2]={0};
	double orient;

	if(mode <4 )sign_x[0]=-1,sign_y[0]= 1,sign_x[1]=1,sign_y[1]=-1;
	else		sign_x[0]=-1,sign_y[0]=-1,sign_x[1]=1,sign_y[1]= 1;

	for(int i=0; i<2; i++)
	{
		if(mode < 4 && i==0)mode=8-mode;
		if(mode > 4 && i==1)mode=8-mode;
		while(true){
			orient=atan2((pixel_y[i]-y),(pixel_x[i]-x));
			orient=fabs(orient);

			if(LEFT[mode]>orient && i==0 || RIGHT[mode]<orient && i==1)
			{
				pixel_x[i]+=(sign_x[i]*1);
				pixel_y[i]=y;
				continue;
			}
			if(LEFT[mode]<=orient && RIGHT[mode]> orient)
			{
				if(m_mark[pixel_y[i]*m_width+pixel_x[i]]==SOURCE)
				{
					weight[i]=sqrt(pow(pixel_y[i]-y,2)+pow(pixel_x[i]-x,2));
					break;
				}
				pixel_x[i]+=(sign_x[i]*1);
				pixel_y[i]=y;
			}else pixel_y[i]+=(sign_y[i]*1);
		}
	}
	for (int i=0; i<2; i++)max+=weight[i];

	for (int i=0; i<2; i++)weight[i]=max-weight[i];

	for( int i = 0; i < 2; i++ )
	{
		temp[0] += m_r[pixel_y[i]*m_width+pixel_x[i]] * weight[i];
		temp[1] += m_g[pixel_y[i]*m_width+pixel_x[i]] * weight[i];
		temp[2] += m_b[pixel_y[i]*m_width+pixel_x[i]] * weight[i];
	}

	for( int i = 0; i < 3; i++ )
		temp[i] /= (weight[0]+weight[1]);

}
void inpainting::HorizontInterpolation(int x, int y, double *temp)
{
	int pixel[PIXEL_POSITION_COUNT] = { x, x, y, y };
	int offset_count[PIXEL_POSITION_COUNT] = { 0, 0, 0, 0 };
	double d_temp=0;
		while( pixel[PIXEL_LEFT] >= 0 )
		{
			if( m_mark[ pixel[PIXEL_LEFT] + m_width*y ] != 0 )
			{
				pixel[PIXEL_LEFT]--;
				offset_count[PIXEL_LEFT]++;
			}
			else
				break;
		}
		pixel[PIXEL_LEFT] = pixel[PIXEL_LEFT] + m_width*y;

		while( pixel[PIXEL_RIGHT] <= m_width )
		{
			if( m_mark[ pixel[PIXEL_RIGHT] + m_width*y ] != 0 )
			{
				pixel[PIXEL_RIGHT]++;
				offset_count[PIXEL_RIGHT]++;
			}
			else
				break;
		}
		pixel[PIXEL_RIGHT] = pixel[PIXEL_RIGHT] + m_width*y;

		double weight[PIXEL_POSITION_COUNT];

		int MAX = offset_count[PIXEL_LEFT] + offset_count[PIXEL_RIGHT];

		weight[PIXEL_LEFT]	= MAX - offset_count[PIXEL_LEFT];
		weight[PIXEL_RIGHT]		= MAX - offset_count[PIXEL_RIGHT];

		for( int i = 0; i < PIXEL_POSITION_COUNT-2; i++ )
		{
			temp[0] += m_r[pixel[i]] * weight[i];
			temp[1] += m_g[pixel[i]] * weight[i];
			temp[2] += m_b[pixel[i]] * weight[i];
		}
		for( int i = 0; i < 3; i++ )
			temp[i] /= (weight[PIXEL_LEFT]+weight[PIXEL_RIGHT]);
}
void inpainting::VerticalInterpolation(int x, int y, double *temp)
{
	int pixel[PIXEL_POSITION_COUNT] = { x, x, y, y };
	int offset_count[PIXEL_POSITION_COUNT] = { 0, 0, 0, 0 };
	double d_temp=0;

		while( pixel[PIXEL_TOP] >= 0 )
		{
			if( m_mark[ pixel[PIXEL_TOP] * m_width + x ] != 0 )
			{
				pixel[PIXEL_TOP]--;
				offset_count[PIXEL_TOP]++;
			}
			else
				break;
		}
		pixel[PIXEL_TOP] = pixel[PIXEL_TOP] * m_width + x;

		while( pixel[PIXEL_BOTTOM] < m_height )
		{
			if( m_mark[ pixel[PIXEL_BOTTOM] * m_width + x ] != 0 )
			{
				pixel[PIXEL_BOTTOM]++;
				offset_count[PIXEL_BOTTOM]++;
			}
			else
				break;
		}
		pixel[PIXEL_BOTTOM] = pixel[PIXEL_BOTTOM] * m_width + x;

		double weight[PIXEL_POSITION_COUNT];

		int MAX = offset_count[PIXEL_TOP] + offset_count[PIXEL_BOTTOM];

		weight[PIXEL_TOP]	= MAX - offset_count[PIXEL_TOP];
		weight[PIXEL_BOTTOM]= MAX - offset_count[PIXEL_BOTTOM];

		for( int i = 2; i < PIXEL_POSITION_COUNT; i++ )
		{
			temp[0] += m_r[pixel[i]] * weight[i];
			temp[1] += m_g[pixel[i]] * weight[i];
			temp[2] += m_b[pixel[i]] * weight[i];
		}
		for( int i = 0; i < 3; i++ )
			temp[i] /= (weight[PIXEL_TOP]+weight[PIXEL_BOTTOM]);
}
DWORD inpainting::getProcessTime()
{
	return m_process_time;
}

void inpainting::magnitude(int g_x, int g_y)
{
	gradient m_grad;

	double magnitude;

	if(m_mark[g_y*m_width+g_x+1] == SOURCE && m_mark[g_y*m_width+g_x-1] == SOURCE && m_mark[(g_y+1)*m_width+g_x] == SOURCE && m_mark[(g_y-1)*m_width+g_x] == SOURCE)
	{
		m_grad = GetColorGradient(g_x,g_y);
		magnitude=m_grad.grad_x*m_grad.grad_x+m_grad.grad_y*m_grad.grad_y;
		magnitude=sqrt(magnitude);

		if(magnitude>check.magnitude)
		{
			check.grad_x = m_grad.grad_x;
			check.grad_y = m_grad.grad_y;
			check.magnitude = magnitude;
			check.check=true;
		}
	}
}
void inpainting::InterpolationThreshold()
{
	int g_count=0;
	double average=0;
	double temp=0;
	gradcheck g_th;
	i_threshold=0;

	for(int j=m_top; j<m_height; j++)
	for(int i=m_left; i<m_width; i++)
	{
		if(m_mark[j*m_width+i]==BOUNDARY)
		{
			g_th=GradCheck(i, j);
			average+=g_th.magnitude;
			g_count++;
		}
	}
	average/=g_count;
	for(int j=m_top; j<m_height; j++)
	for(int i=m_left; i<m_width; i++)
	{
		if(m_mark[j*m_width+i]==BOUNDARY)
		{
			g_th=GradCheck(i, j);
			temp=average-g_th.magnitude;
			i_threshold+=fabs(temp);
		}
	}
	i_threshold/=g_count;
	i_threshold=fabs(average-(i_threshold*0.5));
}