#include "StdAfx.h"

bool inpainting::process(char * name, int searchrange, int min, int max)
{
	Search_Range=searchrange;	
	Tmin=min;			
	Tmax=max;			

	int count=0, pri_x=0, pri_y=0, patch_x=0, patch_y=0;
	double max_pri = 0;
	char path[200]={0};

	double LEFT[9]= {   0 ,   PI/16, 3*PI/16, 5*PI/16, 7*PI/16,  9*PI/16, 11*PI/16, 13*PI/16,15*PI/16}; 
	double RIGHT[9]={PI/16, 3*PI/16, 5*PI/16, 7*PI/16, 9*PI/16, 11*PI/16, 13*PI/16, 15*PI/16,16*PI/16}; 
	sprintf_s(m_InputFileName, sizeof m_InputFileName, "%s",name);

	m_process_time = GetTickCount();
	Convert2Gray(); 
	DrawBoundary(); 
	draw_source();  
	InterpolationThreshold();

	for(int j= m_top; j<=m_bottom; j++)
		for(int i = m_left; i<= m_right; i++)
			if(m_mark[j*m_width+i] == BOUNDARY) m_pri[j*m_width+i] = priority(i,j);

	while( TargetExist() )
	{
		count++;
		max_pri = 0;

		for(int j= m_top; j<=m_bottom; j++)
			for(int i = m_left; i<= m_right; i++)
				if( m_mark[j*m_width+i] == BOUNDARY && m_pri[j*m_width+i] >= max_pri) 
				{
					pri_x = i;
					pri_y = j;
					max_pri = m_pri[j*m_width+i];
				}

		if ( PatchTexture(pri_x, pri_y, patch_x, patch_y, m_threshold)&& m_pri[pri_y*m_width+pri_x] < 1)update(pri_x, pri_y, patch_x,patch_y, ComputeConfidence(pri_x,pri_y));
		else
			updateForInterpolation(pri_x, pri_y,LEFT, RIGHT);


		UpdateBoundary(pri_x, pri_y);
		UpdatePri(pri_x,pri_y);
	}

	m_process_time = GetTickCount() - m_process_time;

	m_PSNR = m_pImage->GetPSNRColor(m_original,m_width,m_height);

	return true;
}

double inpainting::getPSNR()
{
	return m_PSNR;
}

BOOL inpainting::SaveResultImage()
{

	return m_pImage->Save((LPCTSTR)m_InputFileName);
}

BOOL inpainting::SaveResultValue(const char* file_path)
{
	FILE* fp = NULL;
	errno_t error_status = fopen_s(&fp, file_path, "a+");



	if(error_status==0)
	{
		fprintf_s(fp,"=================== %s ===================\n\tPSNR : %0.2f \n\tProcessing Time : %d\n", m_InputFileName, m_PSNR, m_process_time);
	}

	fclose(fp);

	return TRUE;
}