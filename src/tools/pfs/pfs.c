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

static struct pfs root = { 
    .flags = RWX,
    .name = "/",
    .root = {0},
    .brothers = {0},
    .data = NULL
};

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
static void print_pfs(const struct pfs *dir){

    printf("'%s'\n",dir->name );

    struct pfs * embedded;
    list_foreach(&(dir->root), iterator){ 
        embedded = list_item(iterator, struct pfs ,brothers);
        printf(" ├── '%s'\n",embedded->name);
    }
    
}

/** init any already allocated pfs strcutur */
static void init_pfs( struct pfs *new ,char* name, int flags, int type){
    list_init(&new->root);
    list_init(&new->brothers);
    new->flags = flags | type;
    strncpy(new -> name, name, NAME_SIZE-1);
    new->data = NULL;
    new-> size = 0;
}

/** create pfs structure */
static struct pfs *create_pfs(struct pfs *dir, char* name, int flags, int type){
    struct pfs* new = (struct pfs*) malloc(sizeof(struct pfs));
    init_pfs(new, name, flags, type);
    list_addnext(&(dir->root), &(new->brothers));

    return new;
}

static void destroy_dir(struct pfs **elem){
    // exception root
    if (strcmp((*elem)->name, "/") == 0 )
        return ;
    
    list_t* item_unused;
    item_unused = list_unlink(&((*elem)->brothers));
    item_unused = list_unlink(&((*elem)->root));


    free(item_unused);
    *elem = NULL;
}

/** open is here to find on the linked list */
static struct pfs* open_any( char* name ){
    char *last_slash = name;

    // BAD argument value
    if ( name == NULL || last_slash[0] != '/' ){                
        return NULL;
    }

    if(strcmp(last_slash, "/") == 0)
        return &root;

    struct pfs* embedded, *dir = &root;
    char next[NAME_SIZE] = "/" ;

    // Cas general
    while (*last_slash != 0){
        // recherche dans le repertoire
        last_slash += next_slash(last_slash, next);

        list_foreach(&(dir->root), cur){ 
            embedded = list_item(cur, struct pfs, brothers);
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


// Fonction ----------------------------------------------------------------------------------------

struct pfs* opendir_pfs(char *name){
    
    struct pfs* embedded = open_any(name);

    // Verifie que c'est un repertoir
    if (embedded && embedded->flags&&DIR)
        return embedded;
    else
        return NULL; // on peut etre plus flexible
}

// Test --------------------------------------------------------------------------------------------


int main(int argc, char** argv){
    /** init obligatoire il faudrait peut etre une fonction */

    init_pfs(&root , "/", RWX , DIR);
    
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
    struct pfs* fd = open_any("/");
    printf("\t > assert '/' == root is %s \n", &root == fd ? "OUI": "NON");
    printf("\t > fd == NULL is %s \n", NULL == fd ? "OUI": "NON");

    printf("\t======== Create&Open Hello:  \n");
    struct pfs* new = create_pfs(fd, "hello",RW, DIR );
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
    new = create_pfs(&root, "etc",RW, DIR );
    new = create_pfs(&root, "data",RW, DIR );
    new = create_pfs(new, "read.txt",RW, DIR );

    /** print DIR pfs =========================================================================== */
    printf("\n<~/ko6/src/tools/pfs/pfs.c> Print Dir\n");
    print_pfs(&root);

    struct pfs* fd_tree = open_any("/data");
    printf("\t > fd == NULL is %s \n", NULL == fd_tree ? "OUI": "NON");
    print_pfs(fd_tree);

    

    /** open_any pfs ============================================================================ */





    return 0;
}