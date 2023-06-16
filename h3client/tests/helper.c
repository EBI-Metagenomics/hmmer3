#include "helper.h"
#include <stdlib.h>

struct query const ross[2] = {
    [GOOD] = {.name = "tr|Q949S7|Q949S7_ARATH",
              .desc = "NAD(P)-binding Rossmann-fold superfamily protein "
                      "OS=Arabidopsis thaliana OX=3702 GN=At5g15910 PE=1 SV=1",
              .seq =
                  "MLRSLIWKRSQAYSSVVTMSSISQRGNERLLSEVAGSHSRDNKILVLGGNGYVGSHICKE"
                  "ALRQGFSVSSLSRSGRSSLHDSWVDDVTWHQGDLLSPDSLKPALEGITSVISCVGGFGSN"
                  "SQMVRINGTANINAVKAAAEQGVKRFVYISAADFGVINNLIRGYFEGKRATEAEILDKFG"
                  "NRGSVLRPGFIHGTRQVGSIKLPLSLIGAPLEMVLKLLPKEVTKIPVIGPLLIPPVNVKS"
                  "VAATAVKAAVDPEFASGVIDVYRILQHGH",
              .expect = {.nhits = 4, .ln_evalue = -53.808984215028}},
    [BAD] = {.name = ">tr|Q|Q_ARATH",
             .desc = "NAD(P)-binding Rossmann-fold",
             .seq = "MLRSLIWRSQAYSSVVTMSSISQRGNERLLSEVAGSHSRDNKILVLGNGYVGSHICKE"
                    "SQMVRINGTANINAVKAAAEQGVKRFVYISADFGVINNLIRGYFEGKRATEAEILDKF"
                    "NRGSVLRPGFIHGTRQVGSIKLPLSLIGAPLMVLKLLPKEVTKIPVIGPLLIPPVNVS"
                    "TAATAVKAAVDPEFASGVIDVYRILQHGH",
             .expect = {.nhits = 0, .ln_evalue = 0}}};

struct query const corrupt[1] = {
    [0] = {.name = ">a", .desc = "", .seq = "__FF", .expect = {.nhits = 0}}};
