/* ///////////////////////////////////////////////////////////////////// */
/*! 
  \file  
  \brief  Read in a py_heatcool file and compare with known heating and cooling rates

n constant
  
 
  \authors Nick H
  \date    Dec 3 2018
*/
/* ///////////////////////////////////////////////////////////////////// */
#include "pluto.h"

#define LINELENGTH 400



/* ********************************************************************* */
void read_py_heatcool (Data *d, Grid *grid,int flag)
/*!
 * \param [in,out]  VV    a pointer to the PLUTO 3D data array containing
 *                        pimitive variables.
 * \param [in]      dt    the current integration time step
 * \param [in]      Dts   a pointer to the Time_Step structure
 * \param [in]      grid  pointer to an array of Grid structures
 *
 *********************************************************************** */
{
	int i,j,k;
	FILE *fopen (), *fptr;
	char *fgets (), aline[LINELENGTH];
	int ii,jj,nwords;
	double dens,comp_h_pre,comp_c_pre,xray_h_pre,brem_c_pre,line_c_pre;
	double rcen,thetacen;
	int icount;
	#if (BODY_FORCE & VECTOR)
	double gr,gt,gp;
	#endif
	

	if (flag==0) //Initialising first time round
    DOM_LOOP(k,j,i){
    	printf ("Entering py_heatcool - first time round\n");
        
#if COOLING == BLONDIN
		d->comp_h_pre[k][j][i]=1.0;
		d->comp_c_pre[k][j][i]=1.0;
		d->xray_h_pre[k][j][i]=1.0;
		d->line_c_pre[k][j][i]=1.0;
		d->brem_c_pre[k][j][i]=1.0;
#endif
		g_rad[0][k][j][i]=1e-99;
		g_rad[1][k][j][i]=1e-99;
		g_rad[2][k][j][i]=1e-99;	
	}
	else //We will be reading in some updated numbers
	{
		fptr = fopen ("prefactors.dat", "r");
		if (fptr==NULL)
		{
		    DOM_LOOP(k,j,i){
#if COOLING == BLONDIN                
				d->comp_h_pre[k][j][i]=1.0;
				d->comp_c_pre[k][j][i]=1.0;
				d->xray_h_pre[k][j][i]=1.0;
				d->line_c_pre[k][j][i]=1.0;
				d->brem_c_pre[k][j][i]=1.0;
#endif                
    			g_rad[0][k][j][i]=1e-99;
    			g_rad[1][k][j][i]=1e-99;
    			g_rad[2][k][j][i]=1e-99;		
			}
			printf ("NO prefactor file\n");
		}
		else
		{
	    DOM_LOOP(k,j,i){ //Initialise
#if COOLING == BLONDIN                            
			d->comp_h_pre[k][j][i]=-1.0;
			d->comp_c_pre[k][j][i]=-1.0;
			d->xray_h_pre[k][j][i]=-1.0;
			d->line_c_pre[k][j][i]=-1.0;
			d->brem_c_pre[k][j][i]=-1.0;
#endif            
			g_rad[0][k][j][i]=1e-99;
			g_rad[1][k][j][i]=1e-99;
			g_rad[2][k][j][i]=1e-99;
		    }
		fgets (aline, LINELENGTH, fptr);
		icount=0;
		while (fgets (aline, LINELENGTH, fptr) != NULL)	
		{		
			if 
				#if (BODY_FORCE & VECTOR)
				((nwords = sscanf (aline, "%d %le %d %le %le %le %le %le %le %le %le %le %le", &ii, &rcen, &jj, &thetacen, &dens,
				&comp_h_pre, &comp_c_pre, &xray_h_pre, &brem_c_pre, &line_c_pre, &gr, &gt, &gp)) == 13)
				#else
				((nwords = sscanf (aline, "%d %le %d %le %le %le %le %le %le %le",             &ii, &rcen, &jj, &thetacen, &dens,
				&comp_h_pre, &comp_c_pre, &xray_h_pre, &brem_c_pre, &line_c_pre)) == 10)
				#endif
			{
				DOM_LOOP(k,j,i)
				{
					if (fabs(((rcen/UNIT_LENGTH)-grid->x[IDIR][i])/(rcen/UNIT_LENGTH))<1e-5 && fabs((thetacen-grid->x[JDIR][j])/thetacen)<1e-5)
					{
						if ((d->Vc[RHO][k][j][i]*UNIT_DENSITY/dens)-1.>1e-6)
						{
							printf ("Density mismatch in cell i=%i j=%i old=%e new=%e\n",i,j,d->Vc[RHO][k][j][i]*UNIT_DENSITY,dens);
						}
						icount++;
#if COOLING == BLONDIN                                                    
						d->comp_h_pre[k][j][i]=comp_h_pre;
						d->comp_c_pre[k][j][i]=comp_c_pre;
						d->xray_h_pre[k][j][i]=xray_h_pre;
						d->line_c_pre[k][j][i]=line_c_pre;
						d->brem_c_pre[k][j][i]=brem_c_pre;
#endif
						#if (BODY_FORCE & VECTOR)
							g_rad[0][k][j][i]=gr; //The acceleration as computed by python
							g_rad[1][k][j][i]=gt;
							g_rad[2][k][j][i]=gp;
						#endif
					}
				}
			}
			else
			{
				printf ("Prefactor file incorrectly formatted nwords=%i %s\n",nwords,aline);
				exit(0);
			}
		}
#if COOLING == BLONDIN                                                            
	    DOM_LOOP(k,j,i){ //Test
			if (d->comp_h_pre[k][j][i]<0.0)
			{
				printf ("comp_h_prefactor<0.0 for i=%i j=%i comp_h_pre=%e\n",i,j,d->comp_h_pre[k][j][i]);
			}
			if (d->comp_c_pre[k][j][i]<0.0)
			{
				printf ("comp_c_prefactor<0.0 for i=%i j=%i comp_h_pre=%e\n",i,j,d->comp_c_pre[k][j][i]);
			}
			if (d->xray_h_pre[k][j][i]<0.0)
			{
				printf ("xray_h_prefactor<0.0 for i=%i j=%i comp_h_pre=%e\n",i,j,d->xray_h_pre[k][j][i]);
			}
			if (d->line_c_pre[k][j][i]<0.0)
			{
				printf ("line_c_prefactor<0.0 for i=%i j=%i comp_h_pre=%e\n",i,j,d->line_c_pre[k][j][i]);
			}
			if (d->brem_c_pre[k][j][i]<0.0)
			{
				printf ("brem_c_prefactor<0.0 for i=%i j=%i comp_h_pre=%e\n",i,j,d->brem_c_pre[k][j][i]);
			}
		}
#endif
        
    
	
		printf ("Read in %i prefectors\n",icount);
	}
}
}



/*This routine is used when we are running isothermal runs with driving*/

/* ********************************************************************* */
void read_py_iso_temp (Data *d, Grid *grid,int flag)
/*!
 * \param [in,out]  VV    a pointer to the PLUTO 3D data array containing
 *                        pimitive variables.
 * \param [in]      dt    the current integration time step
 * \param [in]      Dts   a pointer to the Time_Step structure
 * \param [in]      grid  pointer to an array of Grid structures
 *
 *********************************************************************** */
{
	int i,j,k;
	FILE *fopen (), *fptr;
	char *fgets (), aline[LINELENGTH];
	int ii,jj,nwords;
	double dens,temp,nh,ne,t_opt,t_UV,t_Xray,r;
	int icount;
	#if (BODY_FORCE & VECTOR)
	double gx,gy,gz;
	#endif
    
    
    double r1=2e12/UNIT_LENGTH;
    double t0=50000;
    double t1=35000; 
    double dtdr; 
    
    dtdr=(t1-t0)/(r1-grid->x[IDIR][0]); 
    
    
    temp=g_inputParam[T_ISO];
	
    printf ("We are restarting so expecting to read in temperatures\n");
    if (flag==1)
    {
        fptr = fopen ("py_pcon_data.dat", "r");
        if (fptr==NULL)
        {
            printf ("NO pcon file\n");
    	}
    	fgets (aline, LINELENGTH, fptr);
    	icount=0;
        k=0;
    	while (fgets (aline, LINELENGTH, fptr) != NULL)	
    	{		
    	    if ((nwords = sscanf (aline, "%d %d %le %le %le %le %le %le %le", &ii, &jj, &temp, &dens, &nh, &ne, &t_opt, &t_UV, &t_Xray)) == 9)
    	    {
                if (fabs((dens-d->Vc[RHO][k][jj+JBEG][ii+IBEG]*UNIT_DENSITY)/dens) > 1e-6)
                    {
                        printf ("NOT SAME  %e %i\n",fabs((dens-d->Vc[RHO][k][jj+JBEG][ii+IBEG]*UNIT_DENSITY)/dens),k);
                    }
                    else
                    {
//                        printf ("Reading into %li %li\n",jj+JBEG,ii+IBEG);
                        py_temp[k][jj+JBEG][ii+IBEG]=temp;
//                        printf ("TEMP READ IN %li %e %e\n",ii+IBEG,grid->x[IDIR][ii+IBEG]*UNIT_LENGTH,py_temp[k][jj+JBEG][ii+IBEG]);
                        
                    }
            }
            else
            {
                printf ("Incorrectly formatted\n");
            }
    	}
        TOT_LOOP(k,j,i) //Fill in the ends of the temperature array
        {
            if (i<IBEG)
            {
                py_temp[k][j][i]=py_temp[k][j][IBEG];
            }
            else if (i>IEND)
            {
                py_temp[k][j][i]=py_temp[k][j][IEND];
            }
        }
    }
    else
    {
    	printf ("First time round so setting temperatures to fixed T\n");
        TOT_LOOP(k,j,i)
        {
            if (grid->x[IDIR][i] < r1)
            {
                py_temp[k][j][i]=t0+(grid->x[IDIR][i]-grid->x[IDIR][0])*dtdr;
            }
            else
            {
                py_temp[k][j][i]=t1;//For the initial run - we have no driving                	
            }
            py_temp[k][j][i]=temp;
//            printf ("%i %e %e\n",i,grid->x[IDIR][i]*UNIT_LENGTH,py_temp[k][j][i]);            
        } 
               	
    }
    
//    printf ("%li %li\n",IBEG,IEND);
    
//    TOT_LOOP(k,j,i)
//    {
//        printf ("We will be using ISO temperatures %i %e %e\n",i,grid->x[IDIR][i]*UNIT_LENGTH,py_temp[k][j][i]);
        
//    }
    
    	
}



/*This routine is used when we are running isothermal runs with driving*/

/* ********************************************************************* */
void read_py_rad_driv (Data *d, Grid *grid,int flag)
/*!
 * \param [in,out]  VV    a pointer to the PLUTO 3D data array containing
 *                        pimitive variables.
 * \param [in]      dt    the current integration time step
 * \param [in]      Dts   a pointer to the Time_Step structure
 * \param [in]      grid  pointer to an array of Grid structures
 *
 *********************************************************************** */
{
	int i,j,k;
	FILE *fopen (), *fptr;
	char *fgets (), aline[LINELENGTH];
	int ii,jj,nwords;
	double rcen,thetacen,dens,gx,gy,gz;
	int icount;

	printf ("Reading in accelerations from python\n");
    if (flag==1)
    {
        fptr = fopen ("py_accelerations.dat", "r");
        if (fptr==NULL)
        {
            printf ("NO accelerations file\n");
    	}
    	fgets (aline, LINELENGTH, fptr);
    	icount=0;
    	while (fgets (aline, LINELENGTH, fptr) != NULL)	
    	{		
    	    if ((nwords = sscanf (aline, "%d %le %d %le %le %le %le %le",  &ii, &rcen, &jj, &thetacen, &dens, &gx, &gy, &gz)) == 8)
    	    {
                if (fabs((dens-d->Vc[RHO][k][jj+JBEG][ii+IBEG]*UNIT_DENSITY)/dens) > 1e-6)
                    printf ("NOT SAME  %e %e %e %e\n",thetacen,dens,gx,d->Vc[RHO][k][jj+JBEG][ii+IBEG]*UNIT_DENSITY);
                else
                {
                    icount++;
    				g_rad[0][k][jj+JBEG][ii+IBEG]=gx/UNIT_ACCELERATION;
    				g_rad[1][k][jj+JBEG][ii+IBEG]=gy/UNIT_ACCELERATION;
    				g_rad[2][k][jj+JBEG][ii+IBEG]=gz/UNIT_ACCELERATION;
                }
                
            }
            else
            {
                printf ("Incorrectly formatted\n");
            }
    	}
        printf ("matched %i accelerations\n",icount);
    }
    else
    {
        DOM_LOOP(k,j,i){
//        	printf ("WE ARE HERE - first time round\n");
    		g_rad[0][k][j][i]=12345678.; //For the initial run - we have no driving - set a flag
    		g_rad[1][k][j][i]=12345678.;
    		g_rad[2][k][j][i]=12345678.;		
    	}        
    }	
}