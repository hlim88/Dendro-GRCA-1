//
// Created by milinda on 9/25/17.
/**
*@author Milinda Fernando
*School of Computing, University of Utah
*@brief Contains the test utilities for the mesh class.
*/
//

#ifndef SFCSORTBENCH_MESHTESTUTILS_H
#define SFCSORTBENCH_MESHTESTUTILS_H

#include "mesh.h"
#include <iostream>
#include <functional>
#include "daUtils.h"
#include "../../BSSN_GR/include/lebedev.h"



namespace ot
{
    namespace test
    {
        /**
         *@brief check whether the getElementalNodal values are correct
         * @param[in] pMesh : pMesh
         * @param[in] vec: zipped vector.
         * @param[in] func: function that is been approximated.
         * @param [in] tol : tolerance
         *
         * */
        template <typename T>
        bool isElementalNodalValuesValid( ot::Mesh * pMesh,T* vec,std::function<T(T,T,T)> func, double tol);


        /**
         * @brief check whether the element to hanging node contribution is computed correctly.
         * Test: Accumilation of the varible vector is equal to the integral of that. Hence we compute the analytical
         * integration and compare it to the computed one.
         * @param[in] pMesh: mesh
         * @param[in] func: infunction to create a varaible.
         * @param[in] Ifunc: integral over the func, over the given octant.
         * @param[in] tol: tolerance value for the check.
         *
         * */
         template<typename T>
         bool isElementalContributionValid(ot::Mesh *pMesh, std::function<T(T,T,T)> func, std::function<T(T,T,T)> Ifunc, double tol);


        /**
         *@brief check whether the unzipped values are correct (Note: Need to unzip after performing the ghost exchange.)
         * @param[in] pMesh : pMesh
         * @param[in] unzipVec: un-zipped vector.
         * @param[in] func: function that is been approximated.
         * @param [in] tol : tolerance
         *
         * */
        template <typename T>
        bool isUnzipValid(ot::Mesh* pMesh, T* unzipVec,std::function<T(T,T,T)> fn, T tol);

        /**
         *@brief check whether the unzip values contains nan in stencil region.
         * @param[in] pMesh : pMesh
         * @param[in] unzipVec: un-zipped vector.
         * @param[in] func: function that is been approximated.
         * @param [in] tol : tolerance
         *
         * */

        template <typename T>
        bool isUnzipNaN(ot::Mesh* pMesh, T* unzipVec);


        /**@brief: Test whether the zip is NaN or not.
         * @param[in] pMesh: input mesh
         * @param[in] zipVec: zip vector.
         * */
         template <typename T>
         bool isZipNAN(ot::Mesh* pMesh, T* zipVec);




        template <typename T>
        bool isUnzipInternalNaN(ot::Mesh* pMesh, T* unzipVec);

        /**
         * @breif : Test whether a known function func, can be correctly interpolated to the surface of a sphere.
         * @param[in] mesh : input mesh
         * @paran[in] zipIn: mesh representation of the function func
         * @param[in] func: testing function
         * @param[in] r: radius of the sphere from (1u<<(maxDepth-1),1u<<(maxDepth-1),1u<<(maxDepth-1))
         * @param[in] tolerance: tolerance for interpolation check.
         *
         * */

        template<typename T>
        bool isInterpToSphereValid(const ot::Mesh * mesh,const T* zipIn,std::function<T(T,T,T)> func,double r,const double *dMin,const double * dMax,double tolerance);




    }
}





// function definitions


template <typename T>
bool ot::test::isElementalNodalValuesValid( ot::Mesh * pMesh,T* vec,std::function<T(T,T,T)> func,double tol)
{

    if(!(pMesh->isActive())) return true;

    std::vector<T> nodalValues;
    nodalValues.resize(pMesh->getNumNodesPerElement());
    const unsigned int eleOrder=pMesh->getElementOrder();

    const std::vector<ot::TreeNode> pNodes=pMesh->getAllElements();
    const std::vector<unsigned int> e2n_dg=pMesh->getE2NMapping_DG();
    unsigned int x,y,z,sz,owner,ix,jy,kz;
    const unsigned int nPe=pMesh->getNumNodesPerElement();
    bool isValid=true;

    for(unsigned int ele=pMesh->getElementLocalBegin();ele<pMesh->getElementLocalEnd();ele++)
    {

        pMesh->getElementNodalValues(vec,&(*(nodalValues.begin())),ele);
        sz=1u<<(m_uiMaxDepth-pNodes[ele].getLevel());
        for(unsigned int k=0;k<(eleOrder+1);k++)
            for(unsigned int j=0;j<(eleOrder+1);j++)
                for(unsigned int i=0;i<(eleOrder+1);i++)
                {
                    x=pNodes[ele].getX()+i*(sz/eleOrder);
                    y=pNodes[ele].getY()+j*(sz/eleOrder);
                    z=pNodes[ele].getZ()+k*(sz/eleOrder);
                    if((i>1 && i<eleOrder) && (j>1 && j<eleOrder) && (k>1 && k<eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[node value Error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==0) && (j==0) && (k>=0 && k<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[left down value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==0) && (j==eleOrder) && (k>=0 && k<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[left up value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==0) && (k==0) && (j>=0 && j<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[left back value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==0) && (k==eleOrder) && (j>=0 && j<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[left front value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==eleOrder) && (j==0) && (k>=0 && k<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[right down value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==eleOrder) && (j==eleOrder) && (k>=0 && k<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[right up value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==eleOrder) && (k==0) && (j>=0 && j<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[right back value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((i==eleOrder) && (k==eleOrder) && (j>=0 && j<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[right front value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((k==0) && (j==0) && (i>=0 && i<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[down back value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((k==eleOrder) && (j==0) && (i>=0 && i<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[down front value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }


                    if((k==0) && (j==eleOrder) && (i>=0 && i<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[up back value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }

                    if((k==eleOrder) && (j==eleOrder) && (i>=0 && i<=eleOrder) && fabs(nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]-func(x,y,z))>tol)
                    {
                        isValid=false;
                        pMesh->dg2eijk(e2n_dg[ele*nPe+k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i],owner,ix,jy,kz);
                        std::cout<<"[up front value error]"<<std::endl;
                        std::cout<<" nvalue: "<<nodalValues[k*(eleOrder+1)*(eleOrder+1)+j*(eleOrder+1)+i]<<" actual: "<<func(x,y,z)<<" ele: "<<ele<<" (i,j,k) : ("<<i<<","<<j<<","<<k<<") : element : "<<pNodes[ele]<<" (x,y,z): ("<<x<<","<<y<<","<<z<<")"<<" owner: "<<pNodes[owner]<<" (i,j,k): ("<<ix<<","<<jy<<","<<kz<<" )"<<std::endl;
                    }


                }

    }


    return isValid;


}



template <typename T>
bool ot::test::isUnzipValid(ot::Mesh* pMesh, T* unzipVec,std::function<T(T,T,T)> fn, T tol)
{

    if(!(pMesh->isActive())) return true;

    const int rank =pMesh->getMPIRank();
    const std::vector<ot::TreeNode> pNodes=pMesh->getAllElements();
    const std::vector<ot::Block> blkList=pMesh->getLocalBlockList();

    /*std::vector<ot::TreeNode> localElem;
    std::vector<ot::TreeNode> ghostElem;
    std::vector<ot::TreeNode> debugBlocks;

    for(unsigned int e=0;e<pNodes.size();e++)
    {
        if(e>=lBegin && e< lEnd)
            localElem.push_back(pNodes[e]);
        else
            ghostElem.push_back(pNodes[e]);

    }


    treeNodesTovtk(localElem,rank,"localElem");
    treeNodesTovtk(ghostElem,rank,"ghostElem");*/

    ot::TreeNode blkNode;
    double pt_min[3];
    double pt_max[3];
    unsigned int regLev;
    unsigned int lx,ly,lz;
    unsigned int hx,hy,hz;
    const unsigned int pW=3;
    unsigned int bflag;
    unsigned int offset;
    unsigned int ib,ie,jb,je,kb,ke;
    double x,y,z;
    bool valid=true;

    for(unsigned int blk=0;blk<blkList.size();blk++)
    {

        blkNode=blkList[blk].getBlockNode();
        regLev=blkList[blk].getRegularGridLev();
        lx=blkList[blk].getAllocationSzX();
        ly=blkList[blk].getAllocationSzY();
        lz=blkList[blk].getAllocationSzZ();

        hx=blkList[blk].computeGridDx();
        hy=blkList[blk].computeGridDy();
        hz=blkList[blk].computeGridDz();

        bflag=blkList[blk].getBlkNodeFlag();
        offset=blkList[blk].getOffset();

        pt_min[0]=(double)blkNode.minX()-pW*hx;
        pt_min[1]=(double)blkNode.minY()-pW*hy;
        pt_min[2]=(double)blkNode.minZ()-pW*hz;

        pt_max[0]=(double)blkNode.maxX()+pW*hx;
        pt_max[1]=(double)blkNode.maxY()+pW*hy;
        pt_max[2]=(double)blkNode.maxZ()+pW*hz;

        ib=pW;
        ie=lx-pW;

        jb=pW;
        je=ly-pW;

        kb=pW;
        ke=lz-pW;

        for(unsigned int k=kb; k < ke; k++)
            for(unsigned int j=jb;j < je; j++)
                for(unsigned int i=ib;i < ie; i++)
                {
                    x=pt_min[0]+i*hx;
                    y=pt_min[1]+j*hy;
                    z=pt_min[2]+k*hz;

                    if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                    {
                        valid=false;
                        std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[internal] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                    }


                }



        if(!(bflag & (1u<<OCT_DIR_LEFT)))
        {
            ib=1;
            ie=pW;
            jb=pW;
            je=ly-pW;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }

                if(!(bflag & (1u<<OCT_DIR_DOWN)))
            {
                ib=1;
                ie=pW;
                jb=1;
                je=pW;
                kb=pW;
                ke=lz-pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT_DOWN] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }



                if(!(bflag & (1u<<OCT_DIR_BACK)))
                {

                    ib=1;
                    ie=pW;
                    jb=1;
                    je=pW;
                    kb=1;
                    ke=pW;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT_DOWN_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }


                }



                if(!(bflag & (1u<<OCT_DIR_FRONT)))
                {

                    ib=1;
                    ie=pW;
                    jb=1;
                    je=pW;
                    kb=lz-pW;
                    ke=lz-1;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT_DOWN_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }


                }



            }

                if(!(bflag & (1u<<OCT_DIR_UP)))
            {
                ib=1;
                ie=pW;
                jb=ly-pW;
                je=ly-1;
                kb=pW;
                ke=lz-pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT_UP] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }




                if(!(bflag & (1u<<OCT_DIR_BACK)))
                {

                    ib=1;
                    ie=pW;
                    jb=ly-pW;
                    je=ly-1;
                    kb=1;
                    ke=pW;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT_UP_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }


                }


                if(!(bflag & (1u<<OCT_DIR_FRONT)))
                {

                    ib=1;
                    ie=pW;
                    jb=ly-pW;
                    je=ly-1;
                    kb=lz-pW;
                    ke=lz-1;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT_UP_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }


                }




            }


                if(!(bflag & (1u<<OCT_DIR_BACK)))
             {
                 ib=1;
                 ie=pW;
                 jb=pW;
                 je=ly-pW;
                 kb=1;
                 ke=pW;

                 for(unsigned int k=kb; k<ke; k++)
                     for(unsigned int j=jb;j<je; j++)
                         for(unsigned int i=ib;i<ie; i++)
                         {
                             x=pt_min[0]+i*hx;
                             y=pt_min[1]+j*hy;
                             z=pt_min[2]+k*hz;

                             if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                             {
                                 valid=false;
                                 std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[LEFT_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<" x: "<<x<<" y: "<<y<<" z: "<<z<<std::endl;
                             }




                         }

             }


                 if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=1;
                ie=pW;
                jb=pW;
                je=ly-pW;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<"  block[LEFT_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }

            }




        }


        if(!(bflag & (1u<<OCT_DIR_RIGHT)))
        {


            ib=lx-pW;
            ie=lx-1;
            jb=pW;
            je=ly-pW;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[RIGHT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }



             if(!(bflag & (1u<<OCT_DIR_DOWN)))
             {
                 ib=lx-pW;
                 ie=lx-1;
                 jb=1;
                 je=pW;
                 kb=pW;
                 ke=lz-pW;

                 for(unsigned int k=kb; k<ke; k++)
                     for(unsigned int j=jb;j<je; j++)
                         for(unsigned int i=ib;i<ie; i++)
                         {
                             x=pt_min[0]+i*hx;
                             y=pt_min[1]+j*hy;
                             z=pt_min[2]+k*hz;

                             if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                             {
                                 valid=false;
                                 std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<"  block[RIGHT_DOWN] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                             }


                         }





                 if(!(bflag & (1u<<OCT_DIR_BACK)))
                 {
                     ib=lx-pW;
                     ie=lx-1;
                     jb=1;
                     je=pW;
                     kb=1;
                     ke=pW;

                     for(unsigned int k=kb; k<ke; k++)
                         for(unsigned int j=jb;j<je; j++)
                             for(unsigned int i=ib;i<ie; i++)
                             {
                                 x=pt_min[0]+i*hx;
                                 y=pt_min[1]+j*hy;
                                 z=pt_min[2]+k*hz;

                                 if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                 {
                                     valid=false;
                                     std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<"  block[RIGHT_DOWN_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                 }


                             }

                 }

                 if(!(bflag & (1u<<OCT_DIR_FRONT)))
                 {
                     ib=lx-pW;
                     ie=lx-1;
                     jb=1;
                     je=pW;
                     kb=lz-pW;
                     ke=lz-1;

                     for(unsigned int k=kb; k<ke; k++)
                         for(unsigned int j=jb;j<je; j++)
                             for(unsigned int i=ib;i<ie; i++)
                             {
                                 x=pt_min[0]+i*hx;
                                 y=pt_min[1]+j*hy;
                                 z=pt_min[2]+k*hz;

                                 if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                 {
                                     valid=false;
                                     std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<"  block[RIGHT_DOWN_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                 }


                             }

                 }



             }

             if(!(bflag & (1u<<OCT_DIR_UP)))
             {
                 ib=lx-pW;
                 ie=lx-1;
                 jb=ly-pW;
                 je=ly-1;
                 kb=pW;
                 ke=lz-pW;

                 for(unsigned int k=kb; k<ke; k++)
                     for(unsigned int j=jb;j<je; j++)
                         for(unsigned int i=ib;i<ie; i++)
                         {
                             x=pt_min[0]+i*hx;
                             y=pt_min[1]+j*hy;
                             z=pt_min[2]+k*hz;

                             if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                             {
                                 valid=false;
                                 std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<"  block[RIGHT_UP] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                             }


                         }


                 if(!(bflag & (1u<<OCT_DIR_BACK)))
                 {
                     ib=lx-pW;
                     ie=lx-1;
                     jb=ly-pW;
                     je=ly-1;
                     kb=1;
                     ke=pW;

                     for(unsigned int k=kb; k<ke; k++)
                         for(unsigned int j=jb;j<je; j++)
                             for(unsigned int i=ib;i<ie; i++)
                             {
                                 x=pt_min[0]+i*hx;
                                 y=pt_min[1]+j*hy;
                                 z=pt_min[2]+k*hz;

                                 if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                 {
                                     valid=false;
                                     std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<"  block[RIGHT_UP_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                 }


                             }

                 }

                 if(!(bflag & (1u<<OCT_DIR_FRONT)))
                 {
                     ib=lx-pW;
                     ie=lx-1;
                     jb=ly-pW;
                     je=ly-1;
                     kb=lz-pW;
                     ke=lz-1;

                     for(unsigned int k=kb; k<ke; k++)
                         for(unsigned int j=jb;j<je; j++)
                             for(unsigned int i=ib;i<ie; i++)
                             {
                                 x=pt_min[0]+i*hx;
                                 y=pt_min[1]+j*hy;
                                 z=pt_min[2]+k*hz;

                                 if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                                 {
                                     valid=false;
                                     std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<"  block[RIGHT_UP_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                 }


                             }

                 }


             }


            if(!(bflag & (1u<<OCT_DIR_BACK)))
            {
                ib=lx-pW;
                ie=lx-1;
                jb=pW;
                je=ly-pW;
                kb=1;
                ke=pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[RIGHT_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<" x: "<<x<<" y: "<<y<<" z: "<<z<<std::endl;
                            }




                        }

            }


            if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=lx-pW;
                ie=lx-1;
                jb=pW;
                je=ly-pW;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[RIGHT_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }

            }







        }


        if(!(bflag & (1u<<OCT_DIR_DOWN)))
        {


            ib=pW;
            ie=lx-pW;
            jb=1;
            je=pW;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[DOWN] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }



            if(!(bflag & (1u<<OCT_DIR_BACK)))
            {
                ib=pW;
                ie=lx-pW;
                jb=1;
                je=pW;
                kb=1;
                ke=pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[DOWN_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<" x: "<<x<<" y: "<<y<<" z: "<<z<<std::endl;
                            }




                        }

            }


           if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=pW;
                ie=lx-pW;
                jb=1;
                je=pW;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[DOWN_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }

            }



        }


        if(!(bflag & (1u<<OCT_DIR_UP)))
        {


            ib=pW;
            ie=lx-pW;
            jb=ly-pW;
            je=ly-1;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[UP] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }



            if(!(bflag & (1u<<OCT_DIR_BACK)))
            {
                ib=pW;
                ie=lx-pW;
                jb=ly-pW;
                je=ly-1;
                kb=1;
                ke=pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[UP_BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<" x: "<<x<<" y: "<<y<<" z: "<<z<<std::endl;
                            }




                        }

            }


            if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=pW;
                ie=lx-pW;
                jb=ly-pW;
                je=ly-1;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[UP_FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }

            }



        }


        if(!(bflag & (1u<<OCT_DIR_BACK)))
        {


            ib=pW;
            ie=lx-pW;
            jb=pW;
            je=ly-pW;
            kb=1;
            ke=pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[BACK] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }

        }


        if(!(bflag & (1u<<OCT_DIR_FRONT)))
        {


            ib=pW;
            ie=lx-pW;
            jb=pW;
            je=ly-pW;
            kb=lz-pW;
            ke=lz-1;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(fabs(unzipVec[offset+k*ly*lx+j*lx+i]-fn(x,y,z))>tol)
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[FRONT] unzip error : unzip value:  "<<unzipVec[offset+k*ly*lx+j*lx+i]<<"\t func(x,y,z): "<<fn(x,y,z)<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }

        }




    }



    return valid;



}


template <typename T>
bool ot::test::isUnzipNaN(ot::Mesh* pMesh, T* unzipVec)
{

    if(!(pMesh->isActive())) return false;

    const int rank =pMesh->getMPIRank();
    const std::vector<ot::TreeNode> pNodes=pMesh->getAllElements();
    const std::vector<ot::Block> blkList=pMesh->getLocalBlockList();

    /*std::vector<ot::TreeNode> localElem;
    std::vector<ot::TreeNode> ghostElem;
    std::vector<ot::TreeNode> debugBlocks;

    for(unsigned int e=0;e<pNodes.size();e++)
    {
        if(e>=lBegin && e< lEnd)
            localElem.push_back(pNodes[e]);
        else
            ghostElem.push_back(pNodes[e]);

    }


    treeNodesTovtk(localElem,rank,"localElem");
    treeNodesTovtk(ghostElem,rank,"ghostElem");*/

    ot::TreeNode blkNode;
    double pt_min[3];
    double pt_max[3];
    unsigned int regLev;
    unsigned int lx,ly,lz;
    unsigned int hx,hy,hz;
    const unsigned int pW=3;
    unsigned int bflag;
    unsigned int offset;
    unsigned int ib,ie,jb,je,kb,ke;
    double x,y,z;
    bool valid=true;

    for(unsigned int blk=0;blk<blkList.size();blk++)
    {

        blkNode=blkList[blk].getBlockNode();
        regLev=blkList[blk].getRegularGridLev();
        lx=blkList[blk].getAllocationSzX();
        ly=blkList[blk].getAllocationSzY();
        lz=blkList[blk].getAllocationSzZ();

        hx=blkList[blk].computeGridDx();
        hy=blkList[blk].computeGridDy();
        hz=blkList[blk].computeGridDz();

        bflag=blkList[blk].getBlkNodeFlag();
        offset=blkList[blk].getOffset();

        pt_min[0]=(double)blkNode.minX()-pW*hx;
        pt_min[1]=(double)blkNode.minY()-pW*hy;
        pt_min[2]=(double)blkNode.minZ()-pW*hz;

        pt_max[0]=(double)blkNode.maxX()+pW*hx;
        pt_max[1]=(double)blkNode.maxY()+pW*hy;
        pt_max[2]=(double)blkNode.maxZ()+pW*hz;

        ib=pW;
        ie=lx-pW;

        jb=pW;
        je=ly-pW;

        kb=pW;
        ke=lz-pW;

        for(unsigned int k=kb; k < ke; k++)
            for(unsigned int j=jb;j < je; j++)
                for(unsigned int i=ib;i < ie; i++)
                {
                    x=pt_min[0]+i*hx;
                    y=pt_min[1]+j*hy;
                    z=pt_min[2]+k*hz;

                    if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                    {
                        valid=false;
                        std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[internal] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                    }


                }



        if(!(bflag & (1u<<OCT_DIR_LEFT)))
        {
            ib=0;
            ie=pW;
            jb=pW;
            je=ly-pW;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }

            if(!(bflag & (1u<<OCT_DIR_DOWN)))
            {
                ib=1;
                ie=pW;
                jb=1;
                je=pW;
                kb=pW;
                ke=lz-pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_down] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }



                if(!(bflag & (1u<<OCT_DIR_BACK)))
                {
                    ib=1;
                    ie=pW;
                    jb=1;
                    je=pW;
                    kb=1;
                    ke=pW;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_down_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }


                if(!(bflag & (1u<<OCT_DIR_FRONT)))
                {
                    ib=1;
                    ie=pW;
                    jb=1;
                    je=pW;
                    kb=lz-pW;
                    ke=lz-1;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_down_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }




            }

            if(!(bflag & (1u<<OCT_DIR_UP)))
            {
                ib=1;
                ie=pW;
                jb=ly-pW;
                je=ly-1;
                kb=pW;
                ke=lz-pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_up] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }


                if(!(bflag & (1u<<OCT_DIR_BACK)))
                {
                    ib=1;
                    ie=pW;
                    jb=ly-pW;
                    je=ly-1;
                    kb=1;
                    ke=pW;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_up_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }


                if(!(bflag & (1u<<OCT_DIR_FRONT)))
                {
                    ib=1;
                    ie=pW;
                    jb=ly-pW;
                    je=ly-1;
                    kb=lz-pW;
                    ke=lz-1;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_up_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }




            }


            if(!(bflag & (1u<<OCT_DIR_BACK)))
            {
                ib=1;
                ie=pW;
                jb=pW;
                je=ly-pW;
                kb=1;
                ke=pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }




                        }

            }


            if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=1;
                ie=pW;
                jb=pW;
                je=ly-pW;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[left_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }

                        }

            }




        }


        if(!(bflag & (1u<<OCT_DIR_RIGHT)))
        {


            ib=lx-pW;
            ie=lx;
            jb=pW;
            je=ly-pW;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }



            if(!(bflag & (1u<<OCT_DIR_DOWN)))
            {
                ib=lx-pW;
                ie=lx-1;
                jb=1;
                je=pW;
                kb=pW;
                ke=lz-pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_down] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }



                if(!(bflag & (1u<<OCT_DIR_BACK)))
                {
                    ib=lx-pW;
                    ie=lx-1;
                    jb=1;
                    je=pW;
                    kb=1;
                    ke=pW;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_down_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }


                if(!(bflag & (1u<<OCT_DIR_FRONT)))
                {
                    ib=lx-pW;
                    ie=lx-1;
                    jb=1;
                    je=pW;
                    kb=lz-pW;
                    ke=lz-1;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_down_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }



            }

            if(!(bflag & (1u<<OCT_DIR_UP)))
            {
                ib=lx-pW;
                ie=lx-1;
                jb=ly-pW;
                je=ly-1;
                kb=pW;
                ke=lz-pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_up] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }



                if(!(bflag & (1u<<OCT_DIR_BACK)))
                {
                    ib=lx-pW;
                    ie=lx-1;
                    jb=ly-pW;
                    je=ly-1;
                    kb=1;
                    ke=pW;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_up_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }


                if(!(bflag & (1u<<OCT_DIR_FRONT)))
                {
                    ib=lx-pW;
                    ie=lx-1;
                    jb=ly-pW;
                    je=ly-1;
                    kb=lz-pW;
                    ke=lz-1;

                    for(unsigned int k=kb; k<ke; k++)
                        for(unsigned int j=jb;j<je; j++)
                            for(unsigned int i=ib;i<ie; i++)
                            {
                                x=pt_min[0]+i*hx;
                                y=pt_min[1]+j*hy;
                                z=pt_min[2]+k*hz;

                                if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                                {
                                    valid=false;
                                    std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_up_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                                }


                            }

                }



            }


            if(!(bflag & (1u<<OCT_DIR_BACK)))
            {
                ib=lx-pW;
                ie=lx-1;
                jb=pW;
                je=ly-pW;
                kb=1;
                ke=pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }




                        }

            }


            if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=lx-pW;
                ie=lx-1;
                jb=pW;
                je=ly-pW;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[right_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }

            }







        }


        if(!(bflag & (1u<<OCT_DIR_DOWN)))
        {


            ib=pW;
            ie=lx-pW;
            jb=0;
            je=pW;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[down] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }



            if(!(bflag & (1u<<OCT_DIR_BACK)))
            {
                ib=pW;
                ie=lx-pW;
                jb=1;
                je=pW;
                kb=1;
                ke=pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[down_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }




                        }

            }


            if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=pW;
                ie=lx-pW;
                jb=1;
                je=pW;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[down_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }

            }



        }


        if(!(bflag & (1u<<OCT_DIR_UP)))
        {


            ib=pW;
            ie=lx-pW;
            jb=ly-pW;
            je=ly;
            kb=pW;
            ke=lz-pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[up] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }



            if(!(bflag & (1u<<OCT_DIR_BACK)))
            {
                ib=pW;
                ie=lx-pW;
                jb=ly-pW;
                je=ly-1;
                kb=1;
                ke=pW;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[up_back] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }




                        }

            }


            if(!(bflag & (1u<<OCT_DIR_FRONT)))
            {
                ib=pW;
                ie=lx-pW;
                jb=ly-pW;
                je=ly-1;

                kb=lz-pW;
                ke=lz-1;

                for(unsigned int k=kb; k<ke; k++)
                    for(unsigned int j=jb;j<je; j++)
                        for(unsigned int i=ib;i<ie; i++)
                        {
                            x=pt_min[0]+i*hx;
                            y=pt_min[1]+j*hy;
                            z=pt_min[2]+k*hz;

                            if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                            {
                                valid=false;
                                std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[up_front] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                            }


                        }

            }



        }


        if(!(bflag & (1u<<OCT_DIR_BACK)))
        {


            ib=pW;
            ie=lx-pW;
            jb=pW;
            je=ly-pW;
            kb=0;
            ke=pW;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[internal] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }

        }


        if(!(bflag & (1u<<OCT_DIR_FRONT)))
        {


            ib=pW;
            ie=lx-pW;
            jb=pW;
            je=ly-pW;
            kb=lz-pW;
            ke=lz;

            for(unsigned int k=kb; k<ke; k++)
                for(unsigned int j=jb;j<je; j++)
                    for(unsigned int i=ib;i<ie; i++)
                    {
                        x=pt_min[0]+i*hx;
                        y=pt_min[1]+j*hy;
                        z=pt_min[2]+k*hz;

                        if(std::isnan(unzipVec[offset+k*ly*lx+j*lx+i]))
                        {
                            valid=false;
                            std::cout<<"rank: "<<rank<<"blk: "<<blkNode<<" block[internal] unzip error : unzip value: NAN "<<" (i,j,k): ("<<i<<", "<<j<<","<<k<<" )"<<"sz: "<<lx<<std::endl;
                        }


                    }

        }




    }



    return (!valid);

}



template <typename T>
bool ot::test::isZipNAN(ot::Mesh* pMesh, T* zipVec)
{
    const unsigned int nodeLocalBegin=pMesh->getNodeLocalBegin();
    const unsigned int nodeLocalEnd=pMesh->getNodeLocalEnd();

    const unsigned int activeRank=pMesh->getMPIRank();

    for(unsigned int i=nodeLocalBegin;i<nodeLocalEnd;i++)
    {
        if(std::isnan(zipVec[i]))
        {
            std::cout<<"rank: "<<activeRank<<" node local: "<<i<<" is nan"<<std::endl;
            assert(false);
        }

    }

}


template <typename T>
bool ot::test::isUnzipInternalNaN(ot::Mesh* pMesh, T* unzipVec)
{


    if(!(pMesh->isActive())) return false;

    const int rank =pMesh->getMPIRank();
    const std::vector<ot::TreeNode> pNodes=pMesh->getAllElements();
    const std::vector<ot::Block> blkList=pMesh->getLocalBlockList();

    /*std::vector<ot::TreeNode> localElem;
    std::vector<ot::TreeNode> ghostElem;
    std::vector<ot::TreeNode> debugBlocks;

    for(unsigned int e=0;e<pNodes.size();e++)
    {
        if(e>=lBegin && e< lEnd)
            localElem.push_back(pNodes[e]);
        else
            ghostElem.push_back(pNodes[e]);

    }


    treeNodesTovtk(localElem,rank,"localElem");
    treeNodesTovtk(ghostElem,rank,"ghostElem");*/

    ot::TreeNode blkNode;
    double pt_min[3];
    double pt_max[3];
    unsigned int regLev;
    unsigned int lx,ly,lz;
    unsigned int hx,hy,hz;
    const unsigned int pW=3;
    unsigned int bflag;
    unsigned int offset;
    unsigned int ib,ie,jb,je,kb,ke;
    double x,y,z;
    bool valid=true;

    for(unsigned int blk=0;blk<blkList.size();blk++) {

        blkNode = blkList[blk].getBlockNode();
        regLev = blkList[blk].getRegularGridLev();
        lx = blkList[blk].getAllocationSzX();
        ly = blkList[blk].getAllocationSzY();
        lz = blkList[blk].getAllocationSzZ();

        hx = blkList[blk].computeGridDx();
        hy = blkList[blk].computeGridDy();
        hz = blkList[blk].computeGridDz();

        bflag = blkList[blk].getBlkNodeFlag();
        offset = blkList[blk].getOffset();

        pt_min[0] = (double) blkNode.minX() - pW * hx;
        pt_min[1] = (double) blkNode.minY() - pW * hy;
        pt_min[2] = (double) blkNode.minZ() - pW * hz;

        pt_max[0] = (double) blkNode.maxX() + pW * hx;
        pt_max[1] = (double) blkNode.maxY() + pW * hy;
        pt_max[2] = (double) blkNode.maxZ() + pW * hz;

        ib = pW;
        ie = lx - pW;

        jb = pW;
        je = ly - pW;

        kb = pW;
        ke = lz - pW;

        for (unsigned int k = kb; k < ke; k++)
            for (unsigned int j = jb; j < je; j++)
                for (unsigned int i = ib; i < ie; i++) {
                    x = pt_min[0] + i * hx;
                    y = pt_min[1] + j * hy;
                    z = pt_min[2] + k * hz;
                    if (std::isnan(unzipVec[offset + k * ly * lx + j * lx + i])) {
                        valid = false;
                        std::cout << "rank: " << rank << "blk: " << blkNode
                                  << " block[internal] unzip error : unzip value: NAN " << " (i,j,k): (" << i << ", "
                                  << j << "," << k << " )" << "sz: " << lx << std::endl;
                    }


                }

    }


    return (!valid);

}


template<typename T>
bool ot::test::isElementalContributionValid(ot::Mesh *pMesh, std::function<T(T,T,T)> func, std::function<T(T,T,T)> Ifunc, double tol)
{



    if(pMesh->isActive())
    {
        const unsigned int rank=pMesh->getMPIRank();
        const unsigned int npes=pMesh->getMPICommSize();

        MPI_Comm commActive=pMesh->getMPICommunicator();

        T * v_func=pMesh->createVector<T>(func);
        T * out =pMesh->createVector<T>((T)0);

        const unsigned int localEleBegin=pMesh->getElementLocalBegin();
        const unsigned int localEleEnd=pMesh->getElementLocalEnd();

        const unsigned int localNodeBegin=pMesh->getNodeLocalBegin();
        const unsigned int localNodeEnd=pMesh->getNodeLocalEnd();

        const unsigned int globalNodeBegin=pMesh->getNodePreGhostBegin();
        const unsigned int globalNodeEnd=pMesh->getNodePostGhostEnd();

        for(unsigned int node=globalNodeBegin;node<globalNodeEnd;node++)
            out[node]=(T)0;


        const unsigned int nPe=pMesh->getNumNodesPerElement();

        T* elemenalVec=new T[nPe];

        pMesh->performGhostExchange(v_func);


        for(unsigned int ele=pMesh->getElementPreGhostBegin();ele<pMesh->getElementPostGhostEnd();ele++)
        {
            pMesh->getElementNodalValues(v_func,elemenalVec,ele);
            pMesh->computeElementalContribution(elemenalVec,out,ele);
        }

        double integralVal=0;
        double integralVal_g=0.0;
        pMesh->performGhostExchange(out);

        for(unsigned int node=localNodeBegin;node<localNodeEnd;node++)
            integralVal+=out[node];

        par::Mpi_Reduce(&integralVal,&integralVal_g,1,MPI_SUM,0,commActive);
        /*char fPrefix[256];
        sprintf(fPrefix,"%s","elecontribution");
        const char * varNames[]={"U"};
        const double * var[]={out};
        io::vtk::mesh2vtuFine(pMesh,fPrefix,0,NULL,NULL,1,varNames,var);*/

        //if(!rank) std::cout<<"integral value: "<<integralVal_g<<std::endl;

        delete [] out;
        delete [] v_func;

    }











}


template<typename T>
bool ot::test::isInterpToSphereValid(const ot::Mesh * mesh,const T* zipIn,std::function<T(T,T,T)> func,double r,const double *dMin,const double * dMax,double tolerance)
{


    const unsigned int rankGlobal=mesh->getMPIRankGlobal();
    const unsigned int npesGlobal=mesh->getMPICommSizeGlobal();
    MPI_Comm commGlobal=mesh->getMPIGlobalCommunicator();

    bool status =true;


    if(mesh->isActive())
    {

        const unsigned int rankActive=mesh->getMPIRank();
        const unsigned int npesActive=mesh->getMPICommSize();
        MPI_Comm  commActive=mesh->getMPICommunicator();

        const unsigned int numPts=LEBEDEV_025_NUM_PTS;

        std::vector<double> coords;
        coords.resize(3*numPts);


        for(unsigned int pts=0;pts<numPts;pts++)
        {
            coords[3*pts + 0]=(r*sin(LEBEDEV_025_THETA[pts])*cos(LEBEDEV_025_PHI[pts])-dMin[0])*(1u<<m_uiMaxDepth)/(dMax[0]-dMin[0]);
            coords[3*pts + 1]=(r*sin(LEBEDEV_025_THETA[pts])*sin(LEBEDEV_025_PHI[pts])-dMin[1])*(1u<<m_uiMaxDepth)/(dMax[1]-dMin[1]);
            coords[3*pts + 2]=(r*cos(LEBEDEV_025_THETA[pts])-dMin[2])*(1u<<m_uiMaxDepth)/(dMax[2]-dMin[2]);

            //std::cout<<"rank: "<<rankActive<<" x: "<<coords[3*pts + 0]<<" y: "<<coords[3*pts + 1]<<" z: "<<coords[3*pts + 2]<<std::endl;

        }

        std::vector<double> interpValues;
        interpValues.resize(numPts);

        std::vector<unsigned int > validIndex;
        ot::da::interpolateToCoords(mesh,zipIn,&(*(coords.begin())),coords.size(),&(*(interpValues.begin())),validIndex);

        unsigned int validSz=validIndex.size();
        unsigned int validSz_g;
        par::Mpi_Reduce(&validSz,&validSz_g,1,MPI_SUM,0,commActive);

        if(!rankActive) std::cout<<"numPts: "<<numPts<<" total valid incices: "<<validSz_g<<std::endl;


        double x,y,z;
        double f_val,f_interp;
        for(unsigned int i=0;i<validIndex.size();i++)
        {
            x=coords[3*validIndex[i]+0];
            y=coords[3*validIndex[i]+1];
            z=coords[3*validIndex[i]+2];

            f_val=func(x,y,z);
            f_interp=interpValues[validIndex[i]];

            if(fabs(f_interp-f_val)>tolerance)
            {
                std::cout<<"rank: "<<rankActive<<" x: "<<x<<" y: "<<y<<" z: "<<z<<" f_val: "<<f_val<<" f_interp: "<<f_interp<<" diff: "<<fabs(f_interp-f_val)<<std::endl;
                status=false;
            }



        }



    }

    return status;




}


#endif //SFCSORTBENCH_MESHTESTUTILS_H
