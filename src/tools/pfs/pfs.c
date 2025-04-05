/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date        2025-03-16
  | / /(     )/ _ \     \copyright   2021 Sorbonne University
  |_\_\ x___x \___/                  https://opensource.org/licenses/MIT

  \file     /tools/pfs/pfs.h
  \author   Lili Zheng
  \brief    pfs a pseudo file system to test the kshell inside Kernel.

  State :   building
  
  NB :      it is a naive version,
            to replace the real file system while the real is not ready.

  // cree un moyen de créé des dossier brother et fils et fichier
  // faire print comme un tree
  // verifier que les paramètre sont legaux notamment les noms et leurs tailles
  //tester opendir et si marche recycler avec open_any

\*------------------------------------------------------------------------------------------------*/

#include <pfs.h>

static struct pfs_s root = { 
    .flags = RWX,
    .name = "/",
    .root = {0},
    .brothers = {0},
    .data = NULL
};

static struct dirent_s root_entry = {
    .d_name = root.name,
    .root = &root,
    .item = &root
};


static struct pfs_s *fileDesciptors[MAX_FD]={0}; // type File

static struct dirent_s *dirDesciptors[MAX_DD]={&root_entry, 0}; // type DIR

/* Outils Static  ------------------------------------------------------------------------------- */

/**
 * charche le prochain mot qui precede par un '/'
 * par exemple si 'hello/char.txt', on a 'hello' dans le param word et
 *      retourne 5 en valeur retourné
 * retourne la taille du mot lue
 */
static int next_slash(char* path, char * word){
    if (path==NULL || *path == 0 ){
        return 0;
    }

    int i,j =0;
    // skip all the meaningless /
    while (*path=='/') path++, j++;

    for ( i = 0; (*(word+i)= *(path+i), *(path+i)!='\0') ; i++ ){
        if ( *(path+i) == '/'){
            *(word+i)= '\0';
            break;
        }
    }

    // skip all the meaningless /
    while (*(path+i)=='/') i++;
    return i+j;
}


/**
 * print the content of the current directory
 */
static void print_pfs(const struct pfs_s *dir){

    printf("'%s'\n",dir->name );

    struct pfs_s * embedded;
    list_foreach(&(dir->root), iterator){ 
        embedded = list_item(iterator, struct pfs_s ,brothers);
        printf(" ├── '%s'\n",embedded->name);
    }
    
}

/** init any already allocated pfs strcutur */
static void init_pfs( struct pfs_s *new ,char* name, int flags, int type){
    list_init(&new->root);
    list_init(&new->brothers);
    new->flags = flags | type;
    strncpy(new -> name, name, NAME_SIZE-1);
    new->data = NULL;
    new-> size = 0;
}

/** create pfs structure */
static struct pfs_s *create_pfs(struct pfs_s *dir, char* name, int flags, int type){
    struct pfs_s* new = (struct pfs_s*) malloc(sizeof(struct pfs_s));
    init_pfs(new, name, flags, type);
    list_addnext(&(dir->root), &(new->brothers));

    return new;
}

static void destroy_dir(struct pfs_s **elem){
    // exception root
    if (strcmp((*elem)->name, "/") == 0 )
        return ;
    
    list_t* item_unused;
    item_unused = list_unlink(&((*elem)->brothers));
    item_unused = list_unlink(&((*elem)->root));


    free(item_unused);
    *elem = NULL;
}

/** oOpen without checkin */
static struct pfs_s* open_any( char* name ){
    char *last_slash = name;

    // BAD argument value
    if ( name == NULL || last_slash[0] != '/' ){                
        return NULL;
    }

    if(strcmp(last_slash, "/") == 0)
        return &root;

    struct pfs_s* embedded, *dir = &root;
    char next[NAME_SIZE] = "/" ;

    // Cas general
    while (*last_slash != 0){
        // recherche dans le repertoire
        last_slash += next_slash(last_slash, next);

        list_foreach(&(dir->root), cur){ 
            embedded = list_item(cur, struct pfs_s, brothers);
            if (*last_slash == 0 && strcmp(embedded->name, next)==0)
                return embedded;
            
            if(strcmp(embedded->name, next)==0){
                dir=embedded;
                cur = (&(dir->root))->next;
                continue;
            }
        }

        //erreur de chemin
        return NULL;
    }
    
    // trouvé
    return embedded;

}

int get_newFd(){
    for(int i =0; i < MAX_FD; i++){
        if(fileDesciptors[i] == NULL)
            return i;
    }
    return -1;
}

struct pfs_s* put_Fd(int fd){
    struct pfs_s* res = fileDesciptors[fd];
    fileDesciptors[fd] = NULL;
    return res;
}

int get_newDd(){
    for(int i =0; i < MAX_FD; i++){
        if(dirDesciptors[i] == NULL)
            return i;
    }
    return -1;
}

struct dirent_s* put_Dd(int fd){
    struct dirent_s* res = dirDesciptors[fd];
    dirDesciptors[fd] = NULL;
    return res;
}


// Fonction ----------------------------------------------------------------------------------------

int open(const char* pathname, int flags){
    return 0;
}

DIR* opendir(char *pathname){
    
    struct pfs_s* embedded = open_any(pathname);

    // Verifie que c'est un repertoir
    if (embedded && embedded->flags&&DIR_T){
        int *res = (int*)malloc(sizeof(int));
        *res = get_newDd();
        dirent_t* new = (dirent_t*) malloc(sizeof(dirent_t));
        new->d_name = pathname;
        new->root = list_item(&embedded->root, pfs_t, root);
        new->item = new->root;
        dirDesciptors[*res] = new;
        return res;
    }
    else{
        return NULL; 
    }
}

void rewinddir(DIR *dirp){
    dirDesciptors[*dirp]->item = dirDesciptors[*dirp]->root;
}

int closedir(DIR *dirp){
    if ( dirp == 0){
        return -1;
    }

    if(dirDesciptors[*dirp] != NULL){
        free((dirDesciptors[*dirp]));
        dirDesciptors[*dirp]=0;
        return 0;
    }

    return -1;
}

struct dirent_s* readdir(DIR *dirp){
    struct dirent_s* res = dirDesciptors[*dirp];

    //return null if end of stream
    struct pfs_s *dir = res->root;
    struct pfs_s *cur = res->item;
    if(&cur->brothers == &dir->root){
        return NULL;
    }

    res->item = list_item(&cur->brothers, pfs_t, brothers);
    return res;
    
}

// Test --------------------------------------------------------------------------------------------


int main(int argc, char** argv){
    /** init obligatoire il faudrait peut etre une fonction */

    init_pfs(&root , "/", RWX , DIR_T);
    
    /** next_slash Test ========================================================================= */
    
    // printf("\n<~/ko6/src/tools/pfs/pfs.c> je m'execute Attention Test Parse name :D\n");
    // char * path ="/" ;
    // // path = "/hello/bonjour/mot/"; //option 1
    // path = "/hello/bonjour/mot/";     //option 2
    // char *it = path;
    // int fileLen = 0;
    // char word[NAME_SIZE];

    // while (*it != '\0') {
    //     printf("\t======== path :   \'%s\' \n", it);
    //     fileLen = next_slash(it, word);
    //     it += fileLen;
    //     //while(*it=='/') it++;
    //     printf("\t > len : %d\n", fileLen);
    //     printf("\t > word is MT from next_slash :  %s\n", strcmp(word, "") == 0 ? "oui" : "non");
    //     printf("\t > path :  %s \n", it);
    //     printf("\t > word from next_slash :  %s \n", word);
    //     usleep(100000);
    // }

    
    /** create pfs ============================================================================== */

    // printf("\n<~/ko6/src/tools/pfs/pfs.c> Create && Open PFS Test \n");

    printf("\t======== Open root :   \'%s\' \n", "/");
    struct pfs_s* fd = open_any("/");
    printf("\t > assert '/' == root is %s \n", &root == fd ? "OUI": "NON");
    printf("\t > fd == NULL is %s \n", NULL == fd ? "OUI": "NON");

    printf("\t======== Create&Open Hello:  \n");
    struct pfs_s* new = create_pfs(fd, "hello",RW, DIR_T );
    fd = open_any("/hello////");
    printf("\t > assert '/hello' == hello is %s \n", fd == new ? "OUI": "NON");
    printf("\t > fd == NULL is %s \n", NULL == fd ? "OUI": "NON");

    printf("\t======== Open bad arg: \n");
    fd = open_any("sds");
    printf("\t > assert '/' == root is %s \n", &root == fd ? "OUI": "NON");
    printf("\t > fd == NULL is %s \n", NULL == fd ? "OUI": "NON");

    printf("\t======== Open mauvais chemin:\n");
    fd = open_any("/sds");
    printf("\t > assert '/' == root is %s \n", new == fd ? "OUI": "NON");
    printf("\t > fd == NULL is %s \n", NULL == fd ? "OUI": "NON");

    // on a confiance a partie de la
    new = create_pfs(&root, "etc",RW, DIR_T );
    new = create_pfs(&root, "data",RW, DIR_T );
    new = create_pfs(new, "read.txt",RW, DIR_T );

    /** print DIR_T pfs =========================================================================== */
    printf("\n<~/ko6/src/tools/pfs/pfs.c> Print Dir\n");
    print_pfs(&root);

    struct pfs_s* fd_tree = open_any("/data");
    printf("\t > fd == NULL is %s \n", NULL == fd_tree ? "OUI": "NON");
    print_pfs(fd_tree);

    

    /** open_any pfs ============================================================================ */





    return 0;
}