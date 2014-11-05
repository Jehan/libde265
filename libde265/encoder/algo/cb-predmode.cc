/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * Authors: struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "libde265/encoder/algo/cb-predmode.h"
#include "libde265/encoder/encoder-context.h"
#include <assert.h>
#include <limits>
#include <math.h>




enc_cb* Algo_CB_PredMode_BruteForce::analyze(encoder_context* ectx,
                                             context_model_table ctxModel,
                                             enc_cb* cb)
{
  // if we try both variants, make a copy of the ctxModel and use the copy for splitting

  bool try_intra = true;
 bool try_inter = (ectx->shdr->slice_type != SLICE_TYPE_I);
  try_inter=false;

  context_model_table ctxInter;

  if (try_inter) {
    copy_context_model_table(ctxInter, ctxModel);
  }


  // try encoding with inter

  enc_cb* cb_inter = NULL;
  enc_cb* cb_intra = NULL;

  if (try_inter) {
    /* TODO
    cb_inter = mIntraPartModeAlgo->analyze(ectx, ctxModel, input,
                                           x0,y0, Log2CbSize, ctDepth, qp);

    cb_inter->PredMode = MODE_INTER;
    */
  }


  // try intra

  if (try_intra) {
    cb->PredMode = MODE_INTRA;
    cb_intra = mIntraAlgo->analyze(ectx, ctxModel, cb);
  }


  // if only one variant has been tested, choose this

  assert(cb_intra || cb_inter);

  if (!try_inter) { return cb_intra; }
  if (!try_intra) { return cb_inter; }


  // compute RD costs for both variants

  const float rd_cost_inter = cb_inter->distortion + ectx->lambda * cb_inter->rate;
  const float rd_cost_intra = cb_intra->distortion + ectx->lambda * cb_intra->rate;

  const bool inter_is_better =  (rd_cost_inter < rd_cost_intra);

  if (inter_is_better) {
    copy_context_model_table(ctxModel, ctxInter);

    // have to reconstruct state
    cb_inter->write_to_image(ectx->img, cb->x,cb->y, true);
    cb_inter->reconstruct(&ectx->accel, ectx->img, cb->x,cb->y);
    return cb_inter;
  }
  else {
    return cb_intra;
  }
}
