// Copyright (c) 2017, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-734707. All Rights
// reserved. See files LICENSE and NOTICE for details.
//
// This file is part of CEED, a collection of benchmarks, miniapps, software
// libraries and APIs for efficient high-order finite element and spectral
// element discretizations for exascale applications. For more information and
// source code availability see http://github.com/ceed.
//
// The CEED research is supported by the Exascale Computing Project 17-SC-20-SC,
// a collaborative effort of two U.S. Department of Energy organizations (Office
// of Science and the National Nuclear Security Administration) responsible for
// the planning and preparation of a capable exascale ecosystem, including
// software, applications, hardware, advanced system engineering and early
// testbed platforms, in support of the nation's exascale computing imperative.

#include "ceed-occa.h"

// *****************************************************************************
// * RESTRICTIONS: Create, Apply, Destroy
// *****************************************************************************
typedef struct {
  const CeedInt *indices;
  CeedInt *indices_allocated;
  occaMemory *device;
  occaKernel kRestrict[6];
} CeedElemRestriction_Occa;

// *****************************************************************************
// * Bytes used
// *****************************************************************************
static inline size_t bytes(const CeedElemRestriction res) {
  return res->nelem * res->elemsize * sizeof(CeedInt);
}

// *****************************************************************************
// * OCCA SYNC functions
// * Ptr == void*, Mem == device
// * occaCopyPtrToMem(occaMemory dest, const void *src,
// * occaCopyMemToPtr(void *dest, occaMemory src,
// *****************************************************************************
static inline void occaSyncH2D(const CeedElemRestriction res) {
  const CeedElemRestriction_Occa *impl = res->data;
  assert(impl);
  assert(impl->device);
  occaCopyPtrToMem(*impl->device, impl->indices, bytes(res), NO_OFFSET, NO_PROPS);
}
static inline void occaSyncD2H(const CeedElemRestriction res) {
  const CeedElemRestriction_Occa *impl = res->data;
  occaCopyMemToPtr((void *)impl->indices, *impl->device, bytes(res), NO_OFFSET,
                   NO_PROPS);
}

// *****************************************************************************
// * OCCA COPY functions
// *****************************************************************************
static inline void occaCopyH2D(const CeedElemRestriction res,
                               const void *from) {
  const CeedElemRestriction_Occa *impl = res->data;
  assert(from);
  assert(impl);
  assert(impl->device);
  occaCopyPtrToMem(*impl->device, from, bytes(res), NO_OFFSET, NO_PROPS);
}
//static inline void occaCopyD2H(const CeedElemRestriction res, void *to) {
//  const CeedElemRestriction_Occa *impl = res->data;
//  assert(to);
//  assert(impl);
//  assert(impl->device);
//  occaCopyMemToPtr(to, *impl->device, bytes(res), NO_OFFSET, NO_PROPS);
//}


// *****************************************************************************
// * CeedElemRestrictionApply_Occa
// *****************************************************************************
static int CeedElemRestrictionApply_Occa(CeedElemRestriction r,
    CeedTransposeMode tmode, CeedInt ncomp,
    CeedTransposeMode lmode, CeedVector u,
    CeedVector v, CeedRequest *request) {
  const CeedElemRestriction_Occa *impl = r->data;
  int ierr;
  const CeedScalar *uu;
  CeedScalar *vv;

  ierr = CeedVectorGetArrayRead(u, CEED_MEM_HOST, &uu); CeedChk(ierr);
  ierr = CeedVectorGetArray(v, CEED_MEM_HOST, &vv); CeedChk(ierr);
  /*
    CeedInt esize = r->nelem*r->elemsize;
    //printf("\n\033[31;1muu(%d):\033[m",u->length); for(int i=0;i<u->length;i++) printf("%f ",uu[i]);

    if (tmode == CEED_NOTRANSPOSE) {
      // Perform: v = r * u
      if (ncomp == 1) {
        for (CeedInt i=0; i<esize; i++) vv[i] = uu[impl->indices[i]];
      } else {
        // vv is (elemsize x ncomp x nelem), column-major
        if (lmode == CEED_NOTRANSPOSE) { // u is (ndof x ncomp), column-major
          for (CeedInt e = 0; e < r->nelem; e++)
            for (CeedInt d = 0; d < ncomp; d++)
              for (CeedInt i=0; i<r->elemsize; i++) {
                vv[i+r->elemsize*(d+ncomp*e)] =
                  uu[impl->indices[i+r->elemsize*e]+r->ndof*d];
              }
        } else { // u is (ncomp x ndof), column-major
          for (CeedInt e = 0; e < r->nelem; e++)
            for (CeedInt d = 0; d < ncomp; d++)
              for (CeedInt i=0; i<r->elemsize; i++) {
                vv[i+r->elemsize*(d+ncomp*e)] =
                  uu[d+ncomp*impl->indices[i+r->elemsize*e]];
              }
        }
      }
    } else {
      // Note: in transpose mode, we perform: v += r^t * u
      if (ncomp == 1) {
        for (CeedInt i=0; i<esize; i++) vv[impl->indices[i]] += uu[i];
      } else {
        // u is (elemsize x ncomp x nelem)
        if (lmode == CEED_NOTRANSPOSE) { // vv is (ndof x ncomp), column-major
          for (CeedInt e = 0; e < r->nelem; e++)
            for (CeedInt d = 0; d < ncomp; d++)
              for (CeedInt i=0; i<r->elemsize; i++) {
                vv[impl->indices[i+r->elemsize*e]+r->ndof*d] +=
                  uu[i+r->elemsize*(d+e*ncomp)];
              }
        } else { // vv is (ncomp x ndof), column-major
          for (CeedInt e = 0; e < r->nelem; e++)
            for (CeedInt d = 0; d < ncomp; d++)
              for (CeedInt i=0; i<r->elemsize; i++) {
                vv[d+ncomp*impl->indices[i+r->elemsize*e]] +=
                  uu[i+r->elemsize*(d+e*ncomp)];
              }
        }
      }
    }
    //printf("\n\033[31;1mvv(%d):\033[m",v->length); for(int i=0;i<v->length;i++) printf("%f ",vv[i]);
    */
  //ierr = CeedVectorRestoreArrayRead(u, &uu); CeedChk(ierr);
  //ierr = CeedVectorRestoreArray(v, &vv); CeedChk(ierr);
  //if (request != CEED_REQUEST_IMMEDIATE) *request = NULL;

  // ***************************************************************************
  const occaMemory indices = *impl->device;
  occaSyncD2H(r);
  //printf("indices: ");for(int i=0;i<(r->nelem*r->elemsize);i++) printf("%d ",impl->indices[i]);

  CeedVector_Occa *u_impl = u->data;
  //printf("\nu2:"); for(int i=0;i<u->length;i++) printf("%f ",u_impl->array[i]);

  CeedVector_Occa *v_impl = v->data;
  //printf("\nv2:"); for(int i=0;i<v->length;i++) printf("%f ",v_impl->array[i]);
  const CeedScalar *us;
  CeedScalar *vs;
  ierr = CeedVectorGetArrayRead(u, CEED_MEM_HOST, &us); CeedChk(ierr);
  ierr = CeedVectorGetArray(v, CEED_MEM_HOST, &vs); CeedChk(ierr);

  //assert(memcmp(u_impl->array,us,u->length)==0); // us == uu
  //assert(memcmp(v_impl->array,vs,v->length)==0);

  const occaMemory ud = *u_impl->device;
  occaMemory vd = *v_impl->device;

  CeedDebug("\033[35m[CeedElemRestriction][Apply] kRestrict");

  if (tmode == CEED_NOTRANSPOSE) {
    // Perform: v = r * u
    if (ncomp == 1) {
      occaKernelRun(impl->kRestrict[0], indices, ud, vd);
    } else {
      // vv is (elemsize x ncomp x nelem), column-major
      if (lmode == CEED_NOTRANSPOSE) {
        // u is (ndof x ncomp), column-major
        occaKernelRun(impl->kRestrict[1], occaInt(ncomp), indices, ud, vd);
      } else {
        // u is (ncomp x ndof), column-major
        occaKernelRun(impl->kRestrict[2], occaInt(ncomp), indices, ud, vd);
      }
    }
  } else {
    // Note: in transpose mode, we perform: v += r^t * u
    if (ncomp == 1) {
      occaKernelRun(impl->kRestrict[3], indices, ud, vd);
    } else {
      // u is (elemsize x ncomp x nelem)
      if (lmode == CEED_NOTRANSPOSE) {
        // vv is (ndof x ncomp), column-major
        occaKernelRun(impl->kRestrict[4], occaInt(ncomp), indices, ud, vd);
      } else {
        // vv is (ncomp x ndof), column-major
        occaKernelRun(impl->kRestrict[5], occaInt(ncomp), indices, ud, vd);
      }
    }
  }

  occaCopyMemToPtr(v_impl->array, vd, v->length*sizeof(CeedScalar), NO_OFFSET,
                   NO_PROPS);

  //CeedDebug("\033[35m[CeedElemRestriction][Apply] occaCopyMemToPtr");
  // Get back Data from device to host
  //ierr = CeedVectorGetArrayRead(u, CEED_MEM_HOST, &us); CeedChk(ierr);
  //printf("\n\033[31;1mus:\033[m"); for(int i=0;i<u->length;i++) printf("%f ",us[i]);
  //ierr = CeedVectorGetArray(v, CEED_MEM_HOST, &vs); CeedChk(ierr);
  //printf("\n\033[31;1mvd:\033[m"); for(int i=0;i<v->length;i++) printf("%f ",vd[i]);

  //occaCopyMemToPtr((void*)us, ud, u->length*sizeof(CeedScalar), NO_OFFSET, NO_PROPS);
  //occaCopyMemToPtr((void*)vs, vd, v->length*sizeof(CeedScalar), NO_OFFSET, NO_PROPS);
  //ierr = CeedVectorGetArray(v, CEED_MEM_HOST, &vs); CeedChk(ierr);
  //printf("\n\033[31;1mvv(%d):\033[m",v->length); for(int i=0;i<v->length;i++) printf("%f ",v_impl->array[i]);

  //assert(memcmp(uu,us,u->length)==0);
  //assert(memcmp(vv,vs,v->length)==0);

  // ***************************************************************************
  ierr = CeedVectorRestoreArrayRead(u, &us); CeedChk(ierr);
  ierr = CeedVectorRestoreArray(v, &vs); CeedChk(ierr);
  if (request != CEED_REQUEST_IMMEDIATE) *request = NULL;
  return 0;

}

// *****************************************************************************
// * CeedElemRestrictionDestroy_Occa
// *****************************************************************************
static int CeedElemRestrictionDestroy_Occa(CeedElemRestriction r) {
  CeedElemRestriction_Occa *impl = r->data;
  int ierr;

  CeedDebug("\033[35m[CeedElemRestriction][Destroy]");
  // free device memory
  // occaMemoryFree(*impl->device);
  // free device object
  //ierr = CeedFree(&impl->device); CeedChk(ierr);
  // free our CeedElemRestriction_Occa struct
  //ierr = CeedFree(&res->data); CeedChk(ierr);
  ierr = CeedFree(&impl->indices_allocated); CeedChk(ierr);
  ierr = CeedFree(&r->data); CeedChk(ierr);
  return 0;
}

// *****************************************************************************
int CeedElemRestrictionCreate_Occa(const CeedElemRestriction r,
                                   const CeedMemType mtype,
                                   const CeedCopyMode cmode,
                                   const CeedInt *indices) {
  const Ceed_Occa *ceed_data=r->ceed->data;
  CeedElemRestriction_Occa *impl;
  int ierr;

  if (mtype != CEED_MEM_HOST)
    return CeedError(r->ceed, 1, "Only MemType = HOST supported");
  // Allocating impl & device **************************************************
  CeedDebug("\033[35m[CeedElemRestriction][Create] Allocating");
  ierr = CeedCalloc(1,&impl); CeedChk(ierr);
  ierr = CeedCalloc(1,&impl->device); CeedChk(ierr);
  *impl->device = occaDeviceMalloc(ceed_data->device, bytes(r), NULL, NO_PROPS);
  r->data = impl;
  // ***************************************************************************
  switch (cmode) {
  case CEED_COPY_VALUES:
    CeedDebug("\t\033[35m[CeedElemRestriction][Create] CEED_COPY_VALUES");
    ierr = CeedMalloc(r->nelem*r->elemsize, &impl->indices_allocated);
    CeedChk(ierr);
    memcpy(impl->indices_allocated, indices, bytes(r));
    impl->indices = impl->indices_allocated;
    break;
  case CEED_OWN_POINTER:
    CeedDebug("\t\033[35m[CeedElemRestriction][Create] CEED_OWN_POINTER");
    impl->indices_allocated = (CeedInt *)indices;
    impl->indices = impl->indices_allocated;
    break;
  case CEED_USE_POINTER:
    CeedDebug("\t\033[35m[CeedElemRestriction][Create] CEED_USE_POINTER");
    impl->indices = indices;
    break;
  default: CeedError(r->ceed,1," OCCA backend no default error");
  }
  CeedDebug("\033[35m[CeedElemRestriction][Create] occaCopyH2D");
  assert(indices);
  occaSyncH2D(r);
  // ***************************************************************************
  CeedDebug("\033[35m[CeedElemRestriction][Create] Building kRestrict");
  occaProperties pKR = occaCreateProperties();
  occaPropertiesSet(pKR, "defines/esize", occaInt(r->nelem*r->elemsize));
  occaPropertiesSet(pKR, "defines/rndof", occaInt(r->ndof));
  occaPropertiesSet(pKR, "defines/rnelem", occaInt(r->nelem));
  occaPropertiesSet(pKR, "defines/relemsize", occaInt(r->elemsize));
  occaPropertiesSet(pKR, "defines/TILE_SIZE", occaInt(TILE_SIZE));
  const occaDevice dev = ceed_data->device;
  char oklPath[4096] = __FILE__;
  const size_t oklPathLen = strlen(oklPath); // path to ceed-occa-restrict.okl
  strcpy(&oklPath[oklPathLen - 2],
         ".okl");  // consider using realpath(3) or something dynamic
  impl->kRestrict[0] = occaDeviceBuildKernel(dev, oklPath, "kRestrict0", pKR);
  impl->kRestrict[1] = occaDeviceBuildKernel(dev, oklPath, "kRestrict1", pKR);
  impl->kRestrict[2] = occaDeviceBuildKernel(dev, oklPath, "kRestrict2", pKR);
  impl->kRestrict[3] = occaDeviceBuildKernel(dev, oklPath, "kRestrict3", pKR);
  impl->kRestrict[4] = occaDeviceBuildKernel(dev, oklPath, "kRestrict4", pKR);
  impl->kRestrict[5] = occaDeviceBuildKernel(dev, oklPath, "kRestrict5", pKR);
  // ***************************************************************************
  r->Apply = CeedElemRestrictionApply_Occa;
  r->Destroy = CeedElemRestrictionDestroy_Occa;
  CeedDebug("\033[35m[CeedElemRestriction][Create] done");
  return 0;
}


// *****************************************************************************
// * TENSORS: Contracts on the middle index
// *          NOTRANSPOSE: V_ajc = T_jb U_abc
// *          TRANSPOSE:   V_ajc = T_bj U_abc
// * CeedScalars are used here, not CeedVectors: we don't touch it yet
// *****************************************************************************
int CeedTensorContract_Occa(Ceed ceed,
                            CeedInt A, CeedInt B, CeedInt C, CeedInt J,
                            const CeedScalar *t, CeedTransposeMode tmode,
                            const CeedScalar *u, CeedScalar *v) {
  CeedInt tstride0 = B, tstride1 = 1;

  //CeedDebug("\033[35m[CeedTensorContract]");
  if (tmode == CEED_TRANSPOSE) {
    tstride0 = 1; tstride1 = J;
  }

  for (CeedInt a=0; a<A; a++) {
    for (CeedInt j=0; j<J; j++) {
      for (CeedInt c=0; c<C; c++)
        v[(a*J+j)*C+c] = 0;
      for (CeedInt b=0; b<B; b++) {
        for (CeedInt c=0; c<C; c++) {
          v[(a*J+j)*C+c] += t[j*tstride0 + b*tstride1] * u[(a*B+b)*C+c];
        }
      }
    }
  }
  return 0;
}
