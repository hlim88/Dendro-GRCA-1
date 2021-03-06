//
// Created by milinda on 11/5/18.
//

/**
 * @brief This file contains the tests to measure the efficiency of the zip/unzip computations.
 * */
#ifndef DENDRO_5_0_ZIPUNZIPTEST_H
#define DENDRO_5_0_ZIPUNZIPTEST_H

#include "mesh.h"
#include "grUtils.h"
#include "grDef.h"
#include "TwoPunctures.h"
#include "rhs.h"
#include "massgrav_constraints.h"


template <typename T>
void applyInitialConditions(const ot::Mesh* mesh, T** zipIn)
{
    unsigned int nodeLookUp_CG;
    unsigned int nodeLookUp_DG;
    unsigned int x,y,z,len;
    const ot::TreeNode * pNodes=&(*(mesh->getAllElements().begin()));
    unsigned int ownerID,ii_x,jj_y,kk_z;
    unsigned int eleOrder=mesh->getElementOrder();
    const unsigned int * e2n_cg=&(*(mesh->getE2NMapping().begin()));
    const unsigned int * e2n_dg=&(*(mesh->getE2NMapping_DG().begin()));
    const unsigned int nPe=mesh->getNumNodesPerElement();
    const unsigned int nodeLocalBegin=mesh->getNodeLocalBegin();
    const unsigned int nodeLocalEnd=mesh->getNodeLocalEnd();


    double* var=new double[massgrav::MASSGRAV_NUM_VARS];

    double mp, mm, mp_adm, mm_adm, E, J1, J2, J3;

    for(unsigned int elem=mesh->getElementLocalBegin();elem<mesh->getElementLocalEnd();elem++)
    {


        for(unsigned int k=0;k<(eleOrder+1);k++)
            for(unsigned int j=0;j<(eleOrder+1);j++ )
                for(unsigned int i=0;i<(eleOrder+1);i++)
                {
                    nodeLookUp_CG=e2n_cg[elem*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i];
                    if(nodeLookUp_CG>=nodeLocalBegin && nodeLookUp_CG<nodeLocalEnd)
                    {
                        nodeLookUp_DG=e2n_dg[elem*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i];
                        mesh->dg2eijk(nodeLookUp_DG,ownerID,ii_x,jj_y,kk_z);
                        len=1u<<(m_uiMaxDepth-pNodes[ownerID].getLevel());
                        x=pNodes[ownerID].getX()+ ii_x*(len/(eleOrder));
                        y=pNodes[ownerID].getY()+ jj_y*(len/(eleOrder));
                        z=pNodes[ownerID].getZ()+ kk_z*(len/(eleOrder));
                        assert(len%eleOrder==0);
                        if (massgrav::MASSGRAV_ID_TYPE == 0) {
                            TwoPunctures((double)x,(double)y,(double)z,var,
                                         &mp, &mm, &mp_adm, &mm_adm, &E, &J1, &J2, &J3);
                        }
                        else if (massgrav::MASSGRAV_ID_TYPE == 1) {
                            massgrav::punctureData((double)x,(double)y,(double)z,var);
                        }
                        else if (massgrav::MASSGRAV_ID_TYPE == 2) {
                            massgrav::KerrSchildData((double)x,(double)y,(double)z,var);
                        }
                        else if (massgrav::MASSGRAV_ID_TYPE == 3) {
                            massgrav::noiseData((double)x,(double)y,(double)z,var);
                        }
                        else {
                            std::cout<<"Unknown ID type"<<std::endl;
                        }
                        for(unsigned int v=0;v<massgrav::MASSGRAV_NUM_VARS;v++)
                            zipIn[v][nodeLookUp_CG]=var[v];


                    }

                }

    }


    for(unsigned int node=mesh->getNodeLocalBegin();node<mesh->getNodeLocalEnd();node++)
        enforce_massgrav_constraints(zipIn,node);


    delete [] var;

    return;

}


#endif //DENDRO_5_0_ZIPUNZIPTEST_H
