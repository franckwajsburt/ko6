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

// Outils Static  ----------------------------------------------------------------------------------

/**
 * charche le prochain mot qui precede par un '/'
 * par exemple si 'hello/char.txt', on a 'hello' dans le param word et
 *      retourne 5 en valeur retourné
 * retourne la taille du mot lue
 */
static int next_slash(const char* path, char * word){
    if (path==NULL){
        *word = '\0';
        return EINVAL;
    }

    for ( int i = 0; *(path+i)!='\0'; i++){
        if ( *(path+i) == '/'){
            *(word+i)= '\0';
            return i ;
        }
        else{
            *(word+i)= *(path+i) ;     
        }
    }
    return 0;
}

/**
 * print the content of the current directory
 */
static void print_pfs(const struct pfs *dir){
    struct pfs * embedded;
    list_foreach(&(dir->brothers), iterator){ 
        embedded = list_item(iterator, struct pfs ,brothers);
        printf("%s\n",embedded->name);
    }
    
}

static void init_pfs( struct pfs *new ,char* name, int flags, int type){
    list_init(&root.root);
    list_init(&root.brothers);
    new->flags = flags | type;
    strncpy(new -> name, name, NAME_SIZE);
    new->data = NULL;
    new-> size = 0;
}

static void create_dir(struct pfs *dir, char* name, int flags, int type){
    struct pfs* new = (struct pfs*) malloc(sizeof(struct pfs));
    list_addnext(&(dir->root), &(new->brothers));
}

static void destroy_dir(struct pfs **elem){
    // exception root
    if (strcmp((*elem)->name, "/") == 0 )
        return ;
    
    list_t* item_unused;
    item_unused = list_unlink(&((*elem)->brothers));
    item_unused = list_unlink(&((*elem)->root));

    //avoid the warning unused
    item_unused++;

    free(*elem);
    *elem = NULL;
}

// static struct pfs* open_any( char* name ){
//     char *last_slash = name;

//     if ( name == NULL || *last_slash != '/' ){      // Cas Root             
//         return NULL;
//     }

//     struct pfs* embedded = &root;

//     // Premier cas (root) traitement special
//     char next[12];
//     int i = next_slash(last_slash, next);

//     // Cas general
//     while (*next != '\0'){
//         last_slash += (1 + i); 
//         // recherche dans le repertoire
//         list_foreach(&(root.root), cur){ 
//             embedded = list_item(cur,struct pfs ,brothers);
//             if (strcmp(embedded->name, next)){
//                 if (embedded->flags|DIR)
//                     continue;
//             }
//         }
//         //erreur de chemin
//         return NULL;
//     }
    
//     // trouvé
//     return embedded;

// }


// Fonction ----------------------------------------------------------------------------------------

struct pfs* opendir_pfs(char *name){
    char *last_slash = name;

    if ( name == NULL || *last_slash != '/' ){      // Cas Root             
        return NULL;
    }

    struct pfs* embedded = &root;

    // Premier cas (root) traitement special
    char next[12];
    int i = next_slash(last_slash, next);


    // Cas general
    while (*next != '\0'){
        last_slash +=(i + 1); 
        // recherche dans le repertoire
        list_foreach(&(root.root), cur){ 
            embedded = list_item(cur,struct pfs ,brothers);
            if (strcmp(embedded->name, next)){
                if (embedded->flags|DIR)
                    continue;
            }
        }
        //erreur de chemin
        return NULL;
    }

    // Verifie que c'est un repertoir
    if (embedded->flags|DIR)
        return embedded;
    else
        return NULL; // on peut etre plus flexible
}

// Test --------------------------------------------------------------------------------------------


int main(int argc, char** argv){
    // init obligatoire il faudrait peut etre une fonction 
    list_init(&root.root);
    list_init(&root.brothers);

    //test
    printf("je m'execute Attention :D\n");
    
    return 0;
}