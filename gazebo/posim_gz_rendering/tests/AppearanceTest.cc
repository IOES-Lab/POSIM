// SPDX-License-Identifier: Apache-2.0
#include "Appearance.hh"
int main(){
 auto a=posim::Defaults();if(!posim::Valid(a))return 1;
 for(unsigned i=2;i<=10;++i){auto bad=a;bad[i]=0.5;if(posim::Valid(bad))return 2;}
 for(auto bad:{-1.,0.5,9007199254740992.}){a[1]=bad;if(posim::Valid(a))return 3;}
 if(posim::WaterPath(-2,-8,10)!=10||posim::WaterPath(2,8,10)!=0||posim::WaterPath(2,-8,10)!=8||posim::WaterPath(-2,8,10)!=2)return 4;
 return 0;
}
