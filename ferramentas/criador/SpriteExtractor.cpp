#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <algorithm>
namespace fs = std::filesystem;

static uint32_t crc32_bytes(const uint8_t* data, size_t n) {
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; ++i) {
        c ^= data[i];
        for (int k=0;k<8;k++) c = (c >> 1) ^ (0xEDB88320u & (-(int32_t)(c & 1)));
    }
    return ~c;
}
static uint32_t adler32_bytes(const uint8_t* data, size_t n) {
    const uint32_t MOD=65521; uint32_t a=1,b=0;
    for(size_t i=0;i<n;i++){ a=(a+data[i])%MOD; b=(b+a)%MOD; }
    return (b<<16)|a;
}
static void put_be32(std::vector<uint8_t>& v, uint32_t x){v.push_back((x>>24)&255);v.push_back((x>>16)&255);v.push_back((x>>8)&255);v.push_back(x&255);} 
static void chunk(std::vector<uint8_t>& png, const char type[4], const std::vector<uint8_t>& data){
    put_be32(png,(uint32_t)data.size()); size_t start=png.size();
    png.insert(png.end(), type, type+4); png.insert(png.end(), data.begin(), data.end());
    uint32_t c=crc32_bytes(png.data()+start,4+data.size()); put_be32(png,c);
}
static bool write_png(const fs::path& path, const std::vector<uint8_t>& rgba){
    const int W=32,H=32; if(rgba.size()!=W*H*4) return false;
    std::vector<uint8_t> raw; raw.reserve(H*(1+W*4));
    for(int y=0;y<H;y++){ raw.push_back(0); raw.insert(raw.end(), rgba.begin()+y*W*4, rgba.begin()+(y+1)*W*4); }
    std::vector<uint8_t> z; z.push_back(0x78); z.push_back(0x01);
    size_t pos=0; while(pos<raw.size()){
        size_t len=std::min<size_t>(65535,raw.size()-pos); bool final=(pos+len==raw.size());
        z.push_back(final?0x01:0x00); uint16_t L=(uint16_t)len, NL=(uint16_t)~L;
        z.push_back(L&255); z.push_back((L>>8)&255); z.push_back(NL&255); z.push_back((NL>>8)&255);
        z.insert(z.end(), raw.begin()+pos, raw.begin()+pos+len); pos+=len;
    }
    uint32_t ad=adler32_bytes(raw.data(),raw.size()); put_be32(z,ad);
    std::vector<uint8_t> png={137,80,78,71,13,10,26,10};
    std::vector<uint8_t> ihdr; put_be32(ihdr,W); put_be32(ihdr,H); ihdr.insert(ihdr.end(),{8,6,0,0,0});
    chunk(png,"IHDR",ihdr); chunk(png,"IDAT",z); chunk(png,"IEND",{});
    std::ofstream out(path,std::ios::binary); out.write((char*)png.data(),png.size()); return !!out;
}
static uint16_t u16(const std::vector<uint8_t>& b,size_t p){return (uint16_t)b[p]|((uint16_t)b[p+1]<<8);} 
static uint32_t u32(const std::vector<uint8_t>& b,size_t p){return (uint32_t)b[p]|((uint32_t)b[p+1]<<8)|((uint32_t)b[p+2]<<16)|((uint32_t)b[p+3]<<24);} 

int main(int argc,char**argv){
    std::cout << "Meu Jogo - Extrator de Sprites 8.60\n";
    fs::path spr = argc>1 ? fs::path(argv[1]) : fs::path("Tibia.spr");
    if(!fs::exists(spr)){
        std::cerr << "Nao encontrei Tibia.spr.\nColoque este EXE ao lado de Tibia.spr ou arraste Tibia.spr sobre o EXE.\n";
        std::cout << "Pressione ENTER para sair..."; std::cin.get(); return 1;
    }
    std::ifstream in(spr,std::ios::binary); std::vector<uint8_t>b((std::istreambuf_iterator<char>(in)),{});
    if(b.size()<12){std::cerr<<"Arquivo SPR invalido.\n";return 2;}
    uint32_t count=u32(b,4); uint64_t tableEnd=8ull+4ull*count;
    if(count==0 || tableEnd>b.size()) { std::cerr<<"Formato SPR nao reconhecido.\n"; return 3; }
    std::cout << "Sprites detectados: " << count << "\n";
    uint32_t first=1,last=count;
    if(argc>2){ first=(uint32_t)std::stoul(argv[2]); last=(argc>3)?(uint32_t)std::stoul(argv[3]):first; }
    else {
        std::cout << "\n1 = Extrair TODOS\n2 = Extrair um intervalo\n3 = Extrair um ID\nEscolha: "; int opt=1; std::cin>>opt;
        if(opt==2){std::cout<<"Primeiro ID: ";std::cin>>first;std::cout<<"Ultimo ID: ";std::cin>>last;}
        else if(opt==3){std::cout<<"ID: ";std::cin>>first;last=first;}
    }
    first=std::max<uint32_t>(1,first); last=std::min<uint32_t>(count,last); if(first>last) std::swap(first,last);
    fs::path outdir=spr.parent_path()/"Sprites_Extraidos"; fs::create_directories(outdir);
    uint32_t ok=0,empty=0,bad=0;
    for(uint32_t id=first;id<=last;id++){
        uint32_t off=u32(b,8ull+(uint64_t)(id-1)*4ull); if(off==0){empty++;continue;}
        if((uint64_t)off+5>b.size()){bad++;continue;}
        uint16_t sz=u16(b,off+3);
        size_t p=(size_t)off+5, end=std::min<size_t>(b.size(),p+sz); std::vector<uint8_t> rgba(32*32*4,0); size_t pix=0;
        while(p+4<=end && pix<1024){ uint16_t trans=u16(b,p), color=u16(b,p+2); p+=4; pix=std::min<size_t>(1024,pix+trans);
            for(uint16_t j=0;j<color && pix<1024;j++){ if(p+3>end){bad++;break;} rgba[pix*4]=b[p];rgba[pix*4+1]=b[p+1];rgba[pix*4+2]=b[p+2];rgba[pix*4+3]=255;p+=3;pix++; }
        }
        std::ostringstream name; name<<"sprite_"<<std::setw(6)<<std::setfill('0')<<id<<".png";
        if(write_png(outdir/name.str(),rgba)) ok++; else bad++;
        if((id-first)%1000==0) std::cout << "\rExtraindo: " << id << "/" << last << std::flush;
        if(id==UINT32_MAX) break;
    }
    std::cout << "\nConcluido. PNGs: "<<ok<<" | vazios: "<<empty<<" | problemas: "<<bad<<"\nPasta: "<<outdir.string()<<"\n";
    std::cout << "Pressione ENTER para sair..."; std::cin.ignore(10000,'\n'); std::cin.get(); return bad?4:0;
}
