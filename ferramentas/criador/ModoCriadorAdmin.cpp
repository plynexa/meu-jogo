#include <sqlite3.h>
#include <filesystem>
#include <iostream>
#include <string>
namespace fs=std::filesystem;

static bool copy_backup(const fs::path& p){
    std::error_code ec; fs::path bak=p; bak += ".antes-admin.bak";
    if(fs::exists(bak)) return true;
    fs::copy_file(p,bak,fs::copy_options::none,ec); return !ec;
}

int main(int argc,char**argv){
    std::cout<<"Meu Jogo - Modo Criador / Admin\n\n";
    fs::path db;
    if(argc>1) db=argv[1];
    else if(fs::exists("meujogo.s3db")) db="meujogo.s3db";
    else if(fs::exists(fs::path("Servidor")/"meujogo.s3db")) db=fs::path("Servidor")/"meujogo.s3db";
    else {
        std::cerr<<"Nao encontrei meujogo.s3db. Coloque o EXE na pasta principal do jogo ou na pasta Servidor.\n";
        std::cin.get(); return 1;
    }

    sqlite3* s=nullptr;
    if(sqlite3_open(db.string().c_str(),&s)!=SQLITE_OK){
        std::cerr<<"Nao consegui abrir o banco. Feche o servidor e tente novamente.\n";
        return 2;
    }
    sqlite3_busy_timeout(s,3000);

    std::cout<<"Banco: "<<db.string()<<"\n";
    std::cout<<"1 = ATIVAR Administrador/GOD no Explorador\n";
    std::cout<<"2 = VOLTAR Explorador para Player normal\n";
    std::cout<<"Escolha: ";
    int op=0; std::cin>>op;
    if(op!=1 && op!=2){sqlite3_close(s);return 0;}

    copy_backup(db);
    int gid=(op==1)?6:1;
    char* err=nullptr;
    if(sqlite3_exec(s,"BEGIN IMMEDIATE;",nullptr,nullptr,&err)!=SQLITE_OK){
        std::cerr<<"Banco ocupado. Feche o servidor antes de alterar.\n";
        if(err) sqlite3_free(err); sqlite3_close(s); return 3;
    }

    std::string q1="UPDATE players SET group_id="+std::to_string(gid)+" WHERE name='Explorador';";
    std::string q2="UPDATE accounts SET group_id="+std::to_string(gid)+" WHERE id=(SELECT account_id FROM players WHERE name='Explorador' LIMIT 1);";

    if(sqlite3_exec(s,q1.c_str(),nullptr,nullptr,&err)!=SQLITE_OK || sqlite3_exec(s,q2.c_str(),nullptr,nullptr,&err)!=SQLITE_OK){
        std::cerr<<"Erro ao atualizar: "<<(err?err:"")<<"\n";
        if(err) sqlite3_free(err);
        sqlite3_exec(s,"ROLLBACK;",nullptr,nullptr,nullptr);
        sqlite3_close(s); return 4;
    }
    sqlite3_exec(s,"COMMIT;",nullptr,nullptr,nullptr);

    sqlite3_stmt* st=nullptr;
    sqlite3_prepare_v2(s,"SELECT p.group_id,a.group_id FROM players p JOIN accounts a ON a.id=p.account_id WHERE p.name='Explorador'",-1,&st,nullptr);
    if(sqlite3_step(st)==SQLITE_ROW)
        std::cout<<"\nExplorador group_id="<<sqlite3_column_int(st,0)<<" | conta group_id="<<sqlite3_column_int(st,1)<<"\n";
    sqlite3_finalize(st); sqlite3_close(s);

    std::cout<<(op==1?"ADMIN/GOD ATIVADO. Inicie o servidor e entre novamente.\n":"Player normal restaurado.\n");
    std::cout<<"Backup criado na primeira alteracao: "<<db.string()<<".antes-admin.bak\n";
    std::cout<<"Pressione ENTER para sair...";
    std::cin.ignore(10000,'\n'); std::cin.get();
    return 0;
}
