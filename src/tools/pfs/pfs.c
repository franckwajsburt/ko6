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
            to replace the real file system for kshell program, while the real one is not ready.

  Repond aux services suivants :
    - open      - opendir
    - close     - closedir
    - read      - readdir
    - write     - rewinddir
    - cd        - ls 
    - mv        - cat
    - cp        - |
    - rm        - > ou 2>
    - mkdir     - < ou 2<
    - touch


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
    .d_name = "/",
    .root = &root,
    .item = &root
};


static struct pfs_s *fileDesciptors[MAX_FD]={0}; // type File

static struct dirent_s *dirDesciptors[MAX_DD]={&root_entry, 0}; // type DIR

// CDW currentlly manage by me
static struct pfs_s *currentDir; // type DIR

/* Outils Static  ------------------------------------------------------------------------------- */

/**
 * charche le prochain mot qui precede par un '/'
 * par exemple si 'hello/char.txt', on a 'hello' dans le param word et
 *      retourne 5 en valeur retourné
 * retourne la taille du mot lue
 */
static int next_slash(const char* path, char * word){
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
 * \brief print the content of the current directory
 */
static void print_pfs(const struct pfs_s *dir){

    printf("'%s'\n",dir->name );

    struct pfs_s * embedded;
    list_foreach(&(dir->root), iterator){ 
        embedded = list_item(iterator, struct pfs_s ,brothers);
        if(dir->root.prev != iterator){
            printf(" ├── '%s'\n",embedded->name);
        }
        else{
            printf(" └── '%s'\n",embedded->name);
        }
    }
    
}

/** init any already allocated pfs structur 
 * * \param new is the pfs_s structure to be initialized
 * * \param name is the name of the file or directory
 * * \param flags is the permission of the file or directory
 * * \param type is the type of the file or directory or executable
*/
static void init_pfs( struct pfs_s *new ,char* name, int flags, int type){
    list_init(&new->root);
    list_init(&new->brothers);
    new->flags = flags | type;
    strncpy(new -> name, name, NAME_SIZE);
    new -> name[NAME_SIZE-1] = '\0';
    new->data = NULL;
    new-> size = 0;
}

/** create pfs structure */
static struct pfs_s *create_pfs(struct pfs_s *dir, char* name, int flags, int type){
    if(dir == NULL || name == NULL || *name == 0){
        return NULL;
    }

    struct pfs_s* new = (struct pfs_s*) malloc(sizeof(struct pfs_s));
    if ( new == NULL){
        return NULL;
    }

    init_pfs(new, name, flags, type);
    list_addnext(&(dir->root), &(new->brothers));

    return new;
}

/**
 *  \brief destroy a pfs structure
 *  \param elem is the pfs_s structure to be destroyed 
 *  \remarks this function is not recursive
 *        it is not possible to destroy the root directory
 *        it is not possible to destroy a directory that contains files
 */
static void destroy_dir(struct pfs_s **elem){
    // exception root
    if (strcmp((*elem)->name, "/") == 0 )
        return ;
    
    list_t* item_unused;

    if ( (*elem)->flags & DIR_T){
        if( list_isempty(&((*elem)->root)) == 0){
            printf("Error: directory '%s' is not empty\n", (*elem)->name);
            return;
        }
    }

    if ( (*elem)->flags & EXEC_T){
            printf("Error: directory '%s' is an executable \n", (*elem)->name);
    }
    
    
    item_unused = list_unlink(&((*elem)->brothers));
    item_unused = list_unlink(&((*elem)->root));

    free(item_unused);
    *elem = NULL;
}

/** Open any pfs from his aboslute name
 * do not touch any reference or create any stuct
 * without any side effect
*/
static struct pfs_s* open_any( const char* name ){
    const char *last_slash = name;

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

/**
 * \return the file descriptor or -1 if no more available
 */
static int get_newFd(){
    for(int i =0; i < MAX_FD; i++){
        if(fileDesciptors[i] == NULL)
            return i;
    }
    return -1;
}

/**
 * make the fd descriptor available
 */
static struct pfs_s* put_Fd(int fd){
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

/**
 * fill the file descriptor table
 * create the file if he does not exist
 * \return the file descriptor or -1 if no more available
 */
int open(const char* pathname, int flags){
    //securité des argument
    if ( pathname == NULL || *pathname == 0 ){
        return -1;
    }

    struct pfs_s * file = open_any(pathname);
    if (file == NULL){                      //create if not found
        //find last '/'
        int path_size = strlen(pathname);
        int i = 0;
        while (*(pathname+path_size-i) != '/') i++;
        
        // prepare name
        char dir[PATH_MAX];
        strncpy(dir, pathname, path_size-i);
        dir[path_size-i] = '\0';

        char new_name[NAME_SIZE];
        strncpy(new_name, pathname+path_size-i+1, NAME_SIZE);
        new_name[NAME_SIZE - 1] = 0;

        struct pfs_s* directory = open_any(dir);
        if(directory == NULL || (directory->flags & DIR_T) != 0){
            return -1;
        }

        // create the file
        file = create_pfs(directory, new_name, RW, FILE_T); // always read and write
        if(file == NULL)
            return -1;
    }
    
    if (file && file->flags & FILE_T){
        int fd = get_newFd();
        fileDesciptors[fd] = file;
        return fd;
    }
    return -1;
}

// make place in the file descriptor table
int close(int fd){
    // verifie que fd est valide
    if ( fd < 0 || fd >= MAX_FD){
        return -1;
    }

    fileDesciptors[fd] = NULL;

    return -1;
}

int _write(int fd, const void *buf, int count){
    //securité des arguments
    if (fd < 0 || fd >= MAX_FD || buf == NULL || count > PAGE_SIZE){
        return -1;
    }

    //verifie que le fd est valide
    struct pfs_s* file = fileDesciptors[fd];
    if (file == NULL || file->flags & DIR_T){
        return -1;
    }

    if (file->data == NULL ){
        file->data = malloc(PAGE_SIZE);
        if (file->data == NULL){
            return -1;
        }
        file->size = 0;  
    }

    memcpy(file->data, buf , count);
    return count; //no way to verify
}

/**
 * \brief read a file
 * \param fd is the file descriptor
 * \param buf is the buffer to put the data
 * \param count is the size of the buffer
 */
int _read(int fd, void *buf, int count){
    //securité des arguments
    if (fd < 0 || fd >= MAX_FD || buf == NULL || count > PAGE_SIZE){
        return -1;
    }

    struct pfs_s* file = fileDesciptors[fd];
    if (file == NULL || !(file->flags & FILE_T )|| file->data == NULL){
        return -1;
    }

    memcpy(buf, file->data, count); //no way to know if he failed

    return count; 
}

/**
 * \brief open direcctory function that return a directory descriptor
 * \param pathname is the absolute name of the file or directory to find in root
 * \return a pointer of the directory descriptor or NULL if the directory is not found
 * 
 * \remarks the return value is weird beacause it respect POSIX API
 */
DIR* opendir(char *pathname){
    // check arguments validity
    if (pathname == NULL || *pathname == 0){
        return NULL;
    }
    
    struct pfs_s* embedded = open_any(pathname);
    if (embedded == NULL){
        return NULL;
    }

    // Verifie que c'est un repertoire
    if (embedded && (embedded->flags & DIR_T)){
        int *res = (int*)malloc(sizeof(int));                   //in order to respect POSIX API
        if(res == NULL){                                        //check malloc
            return NULL;
        }

        *res = get_newDd();
        if (*res < 0){                                          //check descriptor
            free(res);
            return NULL;
        }
        dirent_t* new = (dirent_t*) malloc(sizeof(dirent_t));
        if (new == NULL || res == NULL){
            put_Dd(*res);
            free(res);
            return NULL;
        }

        //initialise the newly created strcuture
        int i = strlen(pathname);
        strncpy(new->d_name, pathname, PATH_MAX);
        new->d_name[i] = '\0';
        new->root = list_item(&embedded->root, pfs_t, root);
        // list_t * fst_elem = list_first(&(new->root->root));
        // new->item = list_item(fst_elem, pfs_t, brothers);
        new->item = new->root;


        // keep his reference in the table
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
        put_Dd(*dirp);
        free(dirp);
        return 0;
    }

    return -1;
}

struct dirent_s* readdir(DIR *dirp){
    if( dirp == 0){
        return NULL;
    }

    struct dirent_s* res = dirDesciptors[*dirp];
    if(res == NULL){
        return NULL;
    }

    //return null if end of stream
    struct pfs_s *dir = res->root;
    struct pfs_s *cur = res->item;
    list_t *next ;

    if( dir == cur ){ // premier cas le decalage est different
        next = cur->root.next;
    }else{
        next = cur->brothers.next;
    }

    if(next == &dir->root){ // si on revient au debut
        return NULL;
    }
    
    cur = list_item(next, pfs_t, brothers);  
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
    new = create_pfs(new, "read.txt",RW, FILE_T);
    new = create_pfs(&root, "data",RW, DIR_T );

    /** print DIR_T pfs ========================================================================= */
    printf("\n<~/ko6/src/tools/pfs/pfs.c> Print Dir\n");
    print_pfs(&root);

    /** open_any pfs ============================================================================ */

    struct pfs_s* fd_tree = open_any("/data");
    printf("\t > fd == NULL is %s \n", NULL == fd_tree ? "OUI": "NON");
    print_pfs(fd_tree);
    fd = open_any("/data");
    printf("\t > fd == NULL is %s \n", NULL == fd ? "OUI": "NON");
    printf("\t > assert '/data' == data is %s \n", fd == new ? "OUI": "NON");

    currentDir = &root;
    /*============================================================================================*\                  
   ┌===================┐   
   .For example        .   cwd = '/'
   .                   .   et le home c'est le root puis voila
   .  /                .   
   .  ├── data         .   
   .  │   └── read.txt .   
   .  ├── etc          .   
   .  └── Hello        .   
   └===================┘   
    \*============================================================================================*/

    // Simulation ls ===============================================================================

    printf("TEST LS =========\n");
    DIR* dir = opendir("/");
    // DIR* dir = currentDir; // avec argument

    if (!dir) {
        printf("ls: cannot open directory\n");
    }

    struct dirent_s* d;
    while ((d = readdir(dir)) != NULL) {
        printf("%s  ", d->item->name);
    }

    printf("\n");
    closedir(dir);

    // Simulation cd ===============================================================================


    return 0;
}