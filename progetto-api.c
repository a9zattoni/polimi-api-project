#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//----------------------------------------------------------------------

#define MAX_CMD_LEN 16  // max lunghezza stringa comando
#define MAX_RES_LEN 256  // max lunghezza stringa risorsa: 255 (max len) + 1 (char terminatore stringa)
#define MAX_PATH_LEN 65536  // max lunghezza stringa path: 255 (max len percorso) * 256 (max len nome risorsa) + 255 ('/' separatore) + 1 (char terminatore stringa)
#define MAX_CONTENT_LEN 258  // max lunghezza contenuto: 255 (max len) + 2 ("") + 1 (char terminatore stringa)
#define MAX_RES_CHILDREN 1024  // max numero nodi figli per ogni directory
#define MAX_OUTPUT_LEN 261  // max lunghezza stringa output: 3 (ok ) + 255 (len max) + 2 ("") + 1 (char terminatore stringa), non è conteggiato l'output del comando find, che stampa dei path

#define CMD_EXIT "exit"
#define CMD_CREATE "create"
#define CMD_CREATE_DIR "create_dir"
#define CMD_READ "read"
#define CMD_WRITE "write"
#define CMD_DELETE "delete"
#define CMD_DELETE_R "delete_r"
#define CMD_FIND "find"
#define CMD_OK "ok"
#define CMD_KO "no"

//----------------------------------------------------------------------

typedef enum {  // stati operazioni con liste
	DLL_STATUS_OK,
	DLL_STATUS_MEM_EXHAUSTED
} dll_status;

typedef enum {  // tipi di nodi usati nelle liste
	DLL_NODE_DIR,
	DLL_NODE_FILE
} dll_node_tag;

typedef void * dll_handle;

typedef struct _dll_node {
	struct _dll_node *prev;
	struct _dll_node *next;
	void *val;
	dll_node_tag tag;
} dll_node;

typedef struct _dll_tag {
	dll_node *head;
	dll_node *tail;
	unsigned int nodes_count;  // per velocizzare operazioni di count degli elementi (aggiornato automaticamente dalle funzioni dll)
} dll_tag;

//----------------------------------------------------------------------

typedef enum {  // stati operazioni con rbt
	RBT_STATUS_OK,
	RBT_STATUS_MEM_EXHAUSTED,
	RBT_STATUS_DUPLICATE_KEY,
	RBT_STATUS_KEY_NOT_FOUND
} rbt_status;

typedef void * rbt_iterator;
typedef void * rbt_handle;

typedef enum { BLACK, RED } node_color;

typedef struct _rbt_node {
	struct _rbt_node *left;
	struct _rbt_node *right;
	struct _rbt_node *parent;
	node_color color;
	void *key;
	void *val;
} rbt_node;

typedef struct _rbt_tag {
	rbt_node *root;
	rbt_node sentinel;
	unsigned int children_count;  // per velocizzare operazioni di count degli elementi (aggiornato automaticamente dalle funzioni rbt)
	int (*compare)( void *a, void *b );
} rbt_tag;

//----------------------------------------------------------------------

typedef struct _fs_dir {
	char *name;
	struct _fs_dir *parent;
	rbt_handle sub_dirs;
	rbt_handle sub_files;
} fs_dir;

typedef struct _fs_file {
	char *name;
	char *content;
	fs_dir *parent;
} fs_file;

//----------------------------------------------------------------------

// implementazione del compareTo per rbt
int compare( void *a, void *b ){
	return strcmp( a, b );
}

//----------------------------------------------------------------------

dll_handle dll_new();
dll_status dll_insert_beginning( dll_handle, void*, dll_node_tag );
dll_status dll_insert_end( dll_handle, void*, dll_node_tag );
dll_status dll_insert_after( dll_handle, dll_node*, void*, dll_node_tag );
dll_status dll_insert_before( dll_handle, dll_node*, void*, dll_node_tag );
void* dll_delete( dll_handle, dll_node* );
void* dll_delete_beginning( dll_handle );
void* dll_delete_end( dll_handle );
void dll_erase( dll_handle );

//----------------------------------------------------------------------

rbt_handle rbt_new( int (*rbt_compare)( void *a, void *b ) );
void rbt_erase_tree( rbt_handle, rbt_node* );
void rbt_erase( rbt_handle );
void rbt_rotate_left( rbt_tag*, rbt_node* );
void rbt_rotate_right(rbt_tag*, rbt_node* );
void rbt_insert_fixup( rbt_tag*, rbt_node* );
rbt_status rbt_insert( rbt_handle, void*, void* );
void rbt_delete_fixup( rbt_tag*, rbt_node* );
rbt_status rbt_delete( rbt_handle, rbt_iterator );
rbt_iterator rbt_next( rbt_handle, rbt_iterator );
rbt_iterator rbt_begin( rbt_handle );
rbt_iterator rbt_end( rbt_handle );
rbt_node* rbt_find( rbt_handle, void* );

//----------------------------------------------------------------------

fs_file* fs_get_sub_file( fs_dir*, char* );
fs_dir* fs_get_sub_dir( fs_dir*, char* );
fs_dir* fs_traverse_tree( fs_dir*, char*, char* );
fs_file* fs_create_file( fs_dir*, char* );
fs_dir* fs_create_dir( fs_dir*, char* );
fs_dir* fs_setup();
char* fs_read_file( fs_file* );
int fs_write_file( fs_file*, char* );
int fs_delete_file( fs_file*, int );
int fs_delete_dir( fs_dir*, int );
void fs_erase_index_r( rbt_handle, rbt_node* );
int fs_erase_index( fs_dir*, rbt_handle );
int fs_delete_dir_r( fs_dir*, int );
int fs_find_r( fs_dir*, char*, dll_tag* );
dll_handle fs_find( fs_dir*, char* );

//----------------------------------------------------------------------

void cmd_create_file(  fs_dir*, char* );
void cmd_create_dir( fs_dir*, char* );
void cmd_read_file( fs_dir*, char* );
void cmd_write_file( fs_dir*, char*, char* );
void cmd_delete( fs_dir*, char* );
void cmd_delete_r( fs_dir*, char* );
void cmd_find( fs_dir*, char* );

//----------------------------------------------------------------------

int main(int argc, char **argv){
	
	char cmd[MAX_CMD_LEN], path[MAX_PATH_LEN], content[MAX_CONTENT_LEN];
	fs_dir *root;
	
	// preparo nuovo file system vouto
	root = fs_setup();
	
	// leggo comandi ed eseguo funzione corrispondente, stampando anche risultato
	// input NON controllato perchè garantito fosse affidabile
	scanf( "%s", cmd );
	while( strcmp( cmd, CMD_EXIT ) != 0 ){
		
		if( strcmp( cmd, CMD_CREATE ) == 0 ){
			scanf( "%s", path );
			cmd_create_file( root, path );
		} else if( strcmp( cmd, CMD_CREATE_DIR ) == 0 ){
			scanf( "%s", path );
			cmd_create_dir( root, path );
		} else if( strcmp( cmd, CMD_READ ) == 0 ){
			scanf( "%s", path );
			cmd_read_file( root, path );
		} else if( strcmp( cmd, CMD_WRITE ) == 0 ){
			scanf( "%s", path );
			scanf( "%s", content );
			cmd_write_file( root, path, content );
		} else if( strcmp( cmd, CMD_DELETE ) == 0 ){
			scanf( "%s", path );
			cmd_delete( root, path );
		} else if( strcmp( cmd, CMD_DELETE_R ) == 0 ){
			scanf( "%s", path );
			cmd_delete_r( root, path );
		} else if( strcmp( cmd, CMD_FIND ) == 0 ){
			scanf( "%s", content );
			cmd_find( root, content );
		}
		
		scanf( "%s", cmd );
	}
	
	// tutto ok, fine
	return 0;
}

//----------------------------------------------------------------------

/**
 * 
 * Esegue comando di creazione file contenuto in $path (composto dal
 * path e dal nome del file) nel file system di radice $root.
 * Stampa risultato dell'opezione.
 * 
 */
void cmd_create_file( fs_dir *root, char *path ){
	
	char res[MAX_RES_LEN];
	fs_dir *dir_parent;
	fs_file *file;
	
	// check dati
	if( root == NULL || path == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dal path, l'ultima dir
	dir_parent = fs_traverse_tree( root, path, res );
	if( dir_parent == NULL || strcmp( res, "" ) == 0 ){  // nome non può essere vuoto
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// creo il file
	file = fs_create_file( dir_parent, res );
	if( file == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// tutto ok
	printf( "%s\n", CMD_OK );
	return;
}

/**
 * 
 * Esegue comando di creazione directory contenuta in $path (composto dal
 * path e dal nome della directory) nel file system di radice $root.
 * Stampa risultato dell'opezione.
 * 
 */
void cmd_create_dir( fs_dir *root, char *path ){
	
	char res[MAX_RES_LEN];
	fs_dir *dir_parent, *dir_child;
	
	// check dati
	if( root == NULL || path == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dal path, l'ultima dir
	dir_parent = fs_traverse_tree( root, path, res );
	if( dir_parent == NULL || strcmp( res, "" ) == 0 ){  // nome non può essere vuoto
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// creo la dir
	dir_child = fs_create_dir( dir_parent, res );
	if( dir_child == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// tutto ok
	printf( "%s\n", CMD_OK );
	return;
}

/**
 * 
 * Esegue comando di lettura contenuto file contenuto in $path
 * (composto dal path e dal nome del file) nel file system di radice
 * $root.
 * Stampa contenuto letto.
 * 
 */
void cmd_read_file( fs_dir *root, char *path ){
	
	char res[MAX_RES_LEN], *file_content_ptr;
	fs_dir *dir_parent;
	fs_file *file;
	
	// check dati
	if( root == NULL || path == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dal path, l'ultima dir
	dir_parent = fs_traverse_tree( root, path, res );
	if( dir_parent == NULL || strcmp( res, "" ) == 0 ){  // nome non può essere vuoto
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dall'ultima dir, il file di cui devo leggere il contenuto
	file = fs_get_sub_file( dir_parent, res );
	if( file == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ne leggo il contenuto
	file_content_ptr = fs_read_file( file );
	if( file_content_ptr == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// tutto ok
	printf( "contenuto %s\n", file_content_ptr );
	return;
}

/**
 * 
 * Esegue comando di scrittura nel file contenuto in $path (composto dal
 * path e dal nome del file) del contenuto $file_content nel file system
 * di radice $root.
 * Stampa risultato dell'opezione.
 * 
 */
void cmd_write_file( fs_dir *root, char *path, char *file_content ){
	
	char res[MAX_RES_LEN];
	fs_dir *dir_parent;
	fs_file *file;
	unsigned int content_len;
	
	// check dati
	if( root == NULL || path == NULL || file_content == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dal path, l'ultima dir
	dir_parent = fs_traverse_tree( root, path, res );
	if( dir_parent == NULL || strcmp( res, "" ) == 0 ){  // nome non può essere vuoto
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dall'ultima dir, il file in cui devo scrivere il contenuto
	file = fs_get_sub_file( dir_parent, res );
	if( file == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// pulisco la stringa dalle ""
	content_len = strlen( file_content ) - 2;  // -2: ""
	memmove( file_content, file_content + sizeof(char), content_len );
	file_content[content_len] = '\0';
	
	// scrivo nel file
	if( fs_write_file( file, file_content ) != 0 ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// tutto ok
	printf( "%s %d\n", CMD_OK, content_len );
	return;
}

/**
 * 
 * Esegue comando di cancellazione file o direcotry contenuta in $path
 * (composto dal path e dal nome del file/directory) nel file system di
 * radice $root.
 * Stampa risultato dell'opezione.
 * 
 */
void cmd_delete( fs_dir *root, char *path ){
	
	char res[MAX_RES_LEN];
	fs_dir *dir_parent, *dir_child;
	fs_file *file;
	
	// check dati
	if( root == NULL || path == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dal path, l'ultima dir
	dir_parent = fs_traverse_tree( root, path, res );
	if( dir_parent == NULL || strcmp( res, "" ) == 0 ){  // nome non può essere vuoto
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// se esiste un file col nome indicato
	file = fs_get_sub_file( dir_parent, res );
	if( file != NULL ){
		
		// lo cancello e sistemo indice associato
		// (operazione non dirty)
		( fs_delete_file( file, 0 ) == 0 ) ? printf( "%s\n", CMD_OK ) : printf( "%s\n", CMD_KO );
		return;
		
	} else{
		
		// se esiste una dir col nome indicato
		dir_child = fs_get_sub_dir( dir_parent, res );
		if( dir_child != NULL ){  // esiste una dir con il nome indicato
			
			// la cancello e sistemo indice associato
			// (operazione non dirty)
			// (controllo perchè potrebbe risultare non cancellabile)
			( fs_delete_dir( dir_child, 0 ) == 0 ) ? printf( "%s\n", CMD_OK ) : printf( "%s\n", CMD_KO );
			return;
			
		} else{
			// non esistono ne file ne dir con il nome indicato
			printf( "%s\n", CMD_KO );
			return;
		}
		
	}
	
}

/**
 * 
 * Esegue comando di cancellazione ricorsiva di file o direcotry
 * contenuti in $path (composto dal path e dal nome del file/directory)
 * nel file system di radice $root.
 * Stampa risultato dell'opezione.
 * 
 */
void cmd_delete_r( fs_dir *root, char *path ){
	
	char res[MAX_RES_LEN];
	fs_dir *dir_parent, *dir_child;
	fs_file *file;
	
	// check dati
	if( root == NULL || path == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// ottengo, dal path, l'ultima dir
	dir_parent = fs_traverse_tree( root, path, res );
	if( dir_parent == NULL || strcmp( res, "" ) == 0 ){  // nome non può essere vuoto
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// se esiste un file col nome indicato
	file = fs_get_sub_file( dir_parent, res );
	if( file != NULL ){
		
		// lo cancello e sistemo indice associato
		// (operazione non dirty)
		fs_delete_file( file, 0 ) == 0 ? printf( "%s\n", CMD_OK ) : printf( "%s\n", CMD_KO );
		return;
		
	} else{
		
		// se esiste una dir col nome indicato
		dir_child = fs_get_sub_dir( dir_parent, res );
		if( dir_child != NULL ){
			
			// la cancello e sistemo indice associato
			// (operazione first call)
			// (operazione non dirty)
			// (controllo perchè potrebbe risultare non cancellabile)
			fs_delete_dir_r( dir_child, 1 ) == 0 ? printf( "%s\n", CMD_OK ) : printf( "%s\n", CMD_KO );
			return;
			
		} else{
			// non esistono ne file ne dir con il nome indicato
			printf( "%s\n", CMD_KO );
			return;
		}
			
	}
}

/**
 * 
 * Esegue comando di ricerca file o direcotry contenuta in $path
 * (composto dal path e dal nome del file/directory) nel file system di
 * radice $root.
 * Stampa tutte occorrenze trovate.
 * 
 */
void cmd_find( fs_dir *root, char *res ){
	
	dll_tag *list_found;
	dll_node *res_node;
	fs_dir *ancestor_node;
	int tmp_len_1, tmp_len_2;
	char *ancestor_name;
	char output[MAX_OUTPUT_LEN];
	
	// check dati
	if( root == NULL || res == NULL ){
		printf( "%s\n", CMD_KO );
		return;
	}
	
	// cerco le risorse
	list_found = fs_find( root, res );
	
	// se non ce ne sono o qualcosa è andato storto
	if( list_found == NULL || list_found->head == NULL ){
		
		printf ( "%s\n", CMD_KO );
		return;
		
	} else{
		
		// itero le risorse trovate
		res_node = list_found->head;
		while( res_node != NULL ){
			
			// ricostruisco il path completo della risorsa
			// formato richiesto per l'output
			
			output[0] = '/';
			
			// inserisco nel buffer il nome della risorsa
			// (operazione splittata in 2 perchè devo riconoscere il tipo di nodo per castarlo, branch speculari)
			if( res_node->tag == DLL_NODE_DIR ){
				// copio nome risorsa
				strcpy( output + sizeof(char), ((fs_dir*)res_node->val)->name );  // offset di 1 char per '/' inserito prima
				// passo al padre
				ancestor_node = ((fs_dir *)res_node->val)->parent;
			} else if( res_node->tag == DLL_NODE_FILE ){
				// copio nome risorsa
				strcpy( output + sizeof(char), ((fs_file*)res_node->val)->name );  // offset di 1 char per '/' inserito prima
				// passo al padre
				ancestor_node = ((fs_file *)res_node->val)->parent;
			}
			
			// risalgo i nodi, fino ad arrivare alla root
			while( ancestor_node != root ){
				
				ancestor_name = ancestor_node->name;
				
				tmp_len_1 = strlen( output );
				tmp_len_2 = strlen( ancestor_name );
				
				// copio il nuovo nome in testa, shiftando in coda quello che già c'era
				memmove( output + tmp_len_2 + sizeof(char), output, tmp_len_1 + sizeof(char) );  // https://stackoverflow.com/a/2328191
				output[0] = '/';
				for( int i = 1; i <= tmp_len_2; i++ ){
					output[i] = ancestor_name[i-1];
				}
				
				// aggiorno il nuovo parent
				ancestor_node = ancestor_node->parent;
			}
			
			printf( "%s %s\n", CMD_OK, output );
			
			res_node = res_node->next;
		}
		
	}
	
	return;
}

//----------------------------------------------------------------------

/**
 * 
 * Ritorna ptr a file figlio di nome $name dalla directory padre puntata
 * da $parent oppure NULL
 * 
 */
fs_file* fs_get_sub_file( fs_dir *parent, char *name ){
	
	rbt_node *sub_file_index;
	
	// check dati
	if( parent == NULL || name == NULL ){
		return NULL;
	}
	
	// cerco nell'indice di parent
	sub_file_index = rbt_find( parent->sub_files, name );
	
	// passo da indice e entry e ritorno ptr
	return ( sub_file_index != NULL ) ? sub_file_index->val : NULL;
}

/**
 * 
 * Ritorna ptr a directory figlia di nome $name dalla directory padre
 * puntata da $parent oppure NULL
 * 
 */
fs_dir* fs_get_sub_dir( fs_dir *parent, char *name ){
	
	rbt_node *sub_dir_index;
	
	// check dati
	if( parent == NULL || name == NULL ){
		return NULL;
	}
	
	// cerco nell'indice di parent
	sub_dir_index = rbt_find( parent->sub_dirs, name );
	
	// passo da indice e entry e ritorno ptr
	return ( sub_dir_index != NULL ) ? sub_dir_index->val : NULL;
}

// ritorna ptr a ultimo layer e in last_layer nome ultima risorsa (che non viene verificata se esiste)
// in caso di layer che non esiste ritorna null e in last_layer nome del layer che non esiteva
// (forse da modificare per copiare anche i token della strtok a ogni iterazione?) TODO
// https://stackoverflow.com/a/3890186
fs_dir* fs_traverse_tree( fs_dir *dir, char *path_ptr, char *last_layer ){
	
	char path[MAX_PATH_LEN];
	fs_dir *current_layer;
	char *layer_1, *layer_2, *layer_3;
	
	// check dati
	if( dir == NULL || path_ptr == NULL || last_layer == NULL ){
		return NULL;
	}
	
	strcpy( path, path_ptr );  // duplico la stringa per lavorarci con la strtok() in quanto che la modifica
	current_layer = dir;
	
	// caso con 1 layer, es: "/"
	layer_1 = strtok( path, "/" );
	if( layer_1 == NULL ){
		strcpy( last_layer, "" );
		return dir;
	}
	
	// caso con 2 layer, es: "/a"
	layer_2 = strtok( NULL, "/" );
	if( layer_2 == NULL ){
		strcpy( last_layer, layer_1 );
		return dir;
	}
	
	// caso con 3+ layer
	if( ( current_layer = fs_get_sub_dir( current_layer, layer_1 ) ) == NULL ){  // controllo se il layer esiste
		strcpy( last_layer, layer_1 );
		return NULL;
	}
	while( ( layer_3 = strtok( NULL, "/" ) ) != NULL ){  // finchè ci sono layer
		
		if( ( current_layer = fs_get_sub_dir( current_layer, layer_2 ) ) == NULL ){  // controllo se il layer esiste
			strcpy( last_layer, layer_2 );
			return NULL;
		}
		
		layer_1 = layer_2;  // avanzo la 'finestra'
		layer_2 = layer_3;
	}
	
	strcpy( last_layer, layer_2 );
	return current_layer;
}

/**
 * 
 * Crea nuovo file vuoto di nome $name e filgio di $parent.
 * Ritorna ptr al file creato oppure NULL.
 * 
 */
fs_file* fs_create_file( fs_dir *parent, char *name ){
	
	fs_file *file;
	char *persistent_name;
	unsigned int sub_files_count, sub_dirs_count;
	
	// check dati
	if( parent == NULL || name == NULL ){
		return NULL;
	}
	
	// controllo che il file possa essere creato
	sub_files_count = ((rbt_tag*)parent->sub_files)->children_count;
	sub_dirs_count = ((rbt_tag*)parent->sub_dirs)->children_count;
	if( sub_files_count + sub_dirs_count > MAX_RES_CHILDREN ){  // raggiungimento limite massimo figli
		return NULL;
	}
	if( rbt_find( parent->sub_dirs, name ) != NULL || rbt_find( parent->sub_files, name ) != NULL ){  // univocità nome risorsa
		return NULL;
	}
	
	// duplico name, per referenziarlo
	persistent_name = (char*)malloc( ( strlen(name) * sizeof(char) ) + ( 1 * sizeof(char) ) );  // +1 per il terminatore stringhe (la strlen() non conteggia)
	if( persistent_name == NULL ){
		// rollback
		return NULL;
	}
	strcpy( persistent_name, name );
	
	// creao file in memoria
	file = (fs_file*)malloc( sizeof(fs_file) );
	if( file == NULL ){
		// rollback
		free( persistent_name );
		return NULL;
	}
	file->name = persistent_name;
	
	// collego file al padre, lato padre
	if( rbt_insert( parent->sub_files, persistent_name, file ) != RBT_STATUS_OK ){  // creo entry nell'indice
		// rollback
		free( file );
		free( persistent_name );
		return NULL;
	}
	
	// collego file al padre, lato figlio
	file->parent = parent;
	
	return file;
}

/**
 * 
 * Crea nuova directory di nome $name e filgia di $parent.
 * Ritorna ptr alla directory creata oppure NULL.
 * 
 */
fs_dir* fs_create_dir( fs_dir *parent, char *name ){
	
	fs_dir *dir;
	rbt_node *index;
	char *persistent_name;
	unsigned int sub_files_count, sub_dirs_count;
	
	// check dati
	if( parent == NULL || name == NULL ){
		return NULL;
	}
	
	// controllo che la dir possa essere creata
	sub_files_count = ((rbt_tag*)parent->sub_files)->children_count;
	sub_dirs_count = ((rbt_tag*)parent->sub_dirs)->children_count;
	if( sub_files_count + sub_dirs_count > MAX_RES_CHILDREN ){  // non raggiungimento limite massimo
		return NULL;
	}
	if( rbt_find( parent->sub_dirs, name ) != NULL || rbt_find( parent->sub_files, name ) != NULL ){  // univocità nome risorsa
		return NULL;
	}
	
	// duplico name, per referenziarlo
	persistent_name = (char *)malloc( ( strlen(name) * sizeof(char) ) + ( 1 * sizeof(char) ) );  // +1 per il terminatore stringhe (la strlen non conteggia)
	if( persistent_name == NULL ){
		// rollback
		return NULL;
	}
	strcpy( persistent_name, name );
	
	// creao dir in memoria
	dir = (fs_dir *)malloc( sizeof(fs_dir) );
	if( dir == NULL ){
		// rollback
		free( persistent_name );
		return NULL;
	}
	dir->name = persistent_name;
	
	// creao e setto indici associati alla dir (sub-dirs e sub-files)
	index = rbt_new( compare );
	if( index == NULL ){
		// rollback
		free( dir );
		free( persistent_name );
		return NULL;
	} else{
		dir->sub_dirs = index;
	}
	index = rbt_new( compare );
	if( index == NULL ){
		// rollback
		rbt_erase( dir->sub_dirs );
		free( dir );
		free( persistent_name );
		return NULL;
	} else{
		dir->sub_files = index;
	}
	
	// collego dir al padre, lato padre
	if( rbt_insert( parent->sub_dirs, persistent_name, dir ) != RBT_STATUS_OK ){
		// rollback
		rbt_erase( dir->sub_dirs );
		rbt_erase( dir->sub_files );
		free( dir );
		free( persistent_name );
		return NULL;
	}
	
	// collego dir al padre, lato figlio
	dir->parent = parent;
	
	return dir;
}

/**
 * 
 * Setta nuovo file system vuoto. Ritorna puntatore alla root oppure NULL.
 * 
 */
fs_dir* fs_setup(){
	
	fs_dir *root;
	rbt_node *index;
	
	// creao dir in memoria
	root = (fs_dir*)malloc( sizeof(fs_dir) );
	if( root == NULL ){
		// rollback
		return NULL;
	}
	// name e parent resteranno a NULL
	
	// creao e setto indici associati alla dir (sub-dirs e sub-files)
	index = rbt_new( compare );
	if( index == NULL ){
		// rollback
		free( root );
		return NULL;
	} else{
		root->sub_dirs = index;
	}
	index = rbt_new( compare );
	if( index == NULL ){
		// rollback
		rbt_erase( root->sub_dirs );
		free( root );
		return NULL;
	} else{
		root->sub_files = index;
	}
	
	return root;
}

/**
 * 
 * Legge e ritorna il contenuto del file puntato da $file, oppure NULL.
 * Se il file è vuoto allora ritorna stringa vuota, non NULL.
 * 
 */
char* fs_read_file( fs_file *file ){
	
	// check dati
	if( file == NULL ){
		return NULL;
	}
	
	return ( file->content != NULL ) ? file->content : "";  // content sarà a NULL se nel file non sono ancora stati scritti dati
}

/**
 * 
 * Scrive nel file $file il contenuto $content, sovrascrivendo.
 * Ritorna 0 se tutto ok, valore negativo in caso di errore.
 * 
 */
int fs_write_file( fs_file *file, char *content ){
	
	char *persistent_content;
	int content_len;
	
	// check dati
	if( file == NULL || content == NULL ){
		return -1;
	}
	
	content_len = strlen( content );
	
	// se nel file non sono ancora stati scritti dati
	if( file->content == NULL ){
		
		// alloco nuova memoria
		persistent_content = (char *)malloc( ( content_len * sizeof(char) ) + ( 1 * sizeof(char) ) );  // +1 per il terminatore stringhe (la strlen non conteggia)
		if( persistent_content == NULL ){
			return -2;
		}
		
	} else{  // se c'erano già dei dati salvati nel file
		
		// se il nuovo contenuto è più grande del vecchio
		if( content_len < strlen( file->content ) ){
			
			// rialloco la memoria
			persistent_content = realloc( file->content, content_len * sizeof(char) + 1 * sizeof(char) );
			if( persistent_content == NULL ){
				return -3;
			}
			// scelta arbitraria di riallocare la memoria solo in caso
			// di spazio insufficiente, e non di riduarla in caso di
			// abbondanza, per limitare le invocazioni di realloc che
			// potrebbe influire negativamente sui tempi di esecuzione.
			// scelta volta a massimizzare il punteggio nei test case
			// piuttosto che ad implementare un "file system" reale.
			
		} else{
			persistent_content = file->content;  // comodità di scrittura di codice su istruzione successiva
		}
		
	}
	
	// salvo il contenuto nel file e collego il ptr del contenuto alla struct file
	strcpy( persistent_content, content );
	file->content = persistent_content;
	
	return 0;
}

/**
 * 
 * Cancella file puntato da $file.
 * Se "operazione sporca" con $dirty_operation=1 allora non vengono
 * aggiornati gli indici che avevano informazioni sul file, col flag ad
 * un altro valore invece sì.
 * Ritorna 0 se tutto ok, altrimenti valore diverso.
 * 
 */
int fs_delete_file( fs_file *file, int dirty_operation ){
	
	rbt_node *file_index;
	
	// check dati
	if( file == NULL ){
		return 1;
	}
	
	// se non devo fare un "operazione sporca"
	if( dirty_operation != 1 ){
		
		// cancello dall'indice del padre
		file_index = rbt_find( file->parent->sub_files, file->name );
		free( file_index->key );  // key punta al name del file, che è allocato dinamicamente
		rbt_delete( file->parent->sub_files, file_index );
		
	}
	
	// elimino file dalla memoria
	free( file->content );  // content è allocato dinamicamente
	free( file );
	// file->name, in caso di operazione sporca, rimane allocata quindi
	// deve essere cancellata successivamente, recuperandola dall'indice
	
	return 0;
}

/**
 * 
 * Cancella directory puntata da $dir ma solo se foglia e non root.
 * Se "operazione sporca" con $dirty_operation=1 allora non vengono
 * aggiornati gli indici che avevano informazioni sulla dir, col flag ad
 * un altro valore invece sì.
 * Ritorna 0 se tutto ok, altrimenti valore diverso.
 * 
 */
int fs_delete_dir( fs_dir *dir, int dirty_operation  ){
	
	rbt_node *dir_index;
	unsigned int sub_dirs_count, sub_files_count;
	
	// check dati
	if( dir == NULL ){
		return 1;
	}
	
	// controllo che la dir possa essere eliminata
	sub_dirs_count = ((rbt_tag*)dir->sub_dirs)->children_count;
	sub_files_count = ((rbt_tag*)dir->sub_files)->children_count;
	if( sub_dirs_count > 0 || sub_files_count > 0 ){  // eliminabile solo se foglia
		return -1;
	}
	if( dir->parent == NULL && dir->name == NULL ){  // root non eliminabile
		return -2;
	}
	
	// se non devo fare un "operazione sporca"
	if( dirty_operation != 1 ){
		
		// cancello dall'indice del padre
		dir_index = rbt_find( dir->parent->sub_dirs, dir->name );
		free( dir_index->key );  // key punta al name della dir, che è allocato dinamicamente
		rbt_delete( dir->parent->sub_dirs, dir_index );
		
	}
	
	// elimino dir e indicie associati dalla memoria
	rbt_erase( dir->sub_dirs );
	rbt_erase( dir->sub_files );
	free( dir );
	// dir->name, in caso di operazione sporca, rimane allocata quindi
	// deve essere cancellata successivamente, recuperandola dall'indice
	
	return 0;
}

/**
 * 
 * Cancella ricorsivamente l'indice $index_handle richiamando ogni volta
 * su $node.
 * Praticamente override di rbt_erase_tree() per cancellare info allocate
 * dinamicamente usate negli indici.
 * 
 */
void fs_erase_index_r( rbt_handle index_handle, rbt_node *node ){
	
	rbt_tag *index = index_handle;
	
	// mi fermo quando arrivo alle foglie
	if( node == &index->sentinel )
		return;
	
	// scendo nel percorso per cancellare dalle foglie
	fs_erase_index_r( index_handle, node->left );
	fs_erase_index_r( index_handle, node->right );
	
	free( node->key );  // corrisponde al name, che è allocato dinamico
	free( node );
}

/**
 * 
 * Cancella l'indice $index_handle della directory $dir.
 * Praticamente override di rbt_erase() per cancellare info allocate
 * dinamicamente usate negli indici.
 * Ritora 0 se tutto ok.
 * 
 */
int fs_erase_index( fs_dir *dir, rbt_handle index_handle ){
	
	rbt_tag *index = index_handle;
	
	// check dati
	if( dir == NULL || index_handle == NULL ){
		return 1;
	}
	
	// cancello indice
	fs_erase_index_r( index, index->root );
	free( index );
	
	return 0;
}

/**
 * 
 * Cancella ricorsivamente la directory $dir e tutti i suoi discendenti.
 * Alla prima invocazione $first_call dovrà essere settato a 1, mentre
 * nelle invocazioni ricorsive successive il flag verrà settato a 0.
 * Ritorna 0 se tutto ok, valore diverso in base all'errore.
 * 
 */
int fs_delete_dir_r( fs_dir *dir, int first_call ){
	
	rbt_iterator *sub_dir_index, *sub_file_index;
	rbt_node *dir_index;
	fs_dir *sub_dir_ptr;
	fs_file *sub_file_ptr;
	
	// check dati
	if( dir == NULL ){
		return 1;
	}
	
	// controllo che la dir possa essere eliminata
	if( dir->parent == NULL && dir->name == NULL ){  // root non eliminabile
		return -2;
	}
	
	//  itero sub-dirs, tramite indice
	sub_dir_index = rbt_begin( dir->sub_dirs );
	while( sub_dir_index != NULL ){
		
		// passo dall'indice alla vera sotto dir
		sub_dir_ptr = ((rbt_node*)sub_dir_index)->val;
		
		// se non è foglia
		if( ((rbt_tag*)sub_dir_ptr->sub_dirs)->children_count > 0 ){
			// richiamo ricorsivamente su di essa
			fs_delete_dir_r( sub_dir_ptr, 0 );
		}
		
		// passo alla sotto dir successiva
		sub_dir_index = rbt_next( dir->sub_dirs, sub_dir_index );
	}
	
	//  itero sotto file, tramite indice
	sub_file_index = rbt_begin( dir->sub_files );
	while( sub_file_index != NULL ){
		
		// passo dall'indice al vero sotto file
		sub_file_ptr = ((rbt_node*)sub_file_index)->val;
		
		// lo cancello
		fs_delete_file( sub_file_ptr, 1 );  // cancellazione dirty per evitare il fixup del rbt che tanto dovrà essere cancellato
		
		// passo al sotto file successivo
		sub_file_index = rbt_next( dir->sub_files, sub_file_index );
	}
	
	// cancello indicei
	fs_erase_index( dir, dir->sub_files );
	fs_erase_index( dir, dir->sub_dirs );
	
	// se è la dir su cui è stata fatta l'invocazione originale
	if( first_call == 1 ){
		// cancello dall'indice del padre
		// (come se fosse un'operazione pulita)
		dir_index = rbt_find( dir->parent->sub_dirs, dir->name );
		free( dir_index->key );  // key punta al name della dir, che è allocato dinamicamente
		rbt_delete( dir->parent->sub_dirs, dir_index );
	}
	// elimino dir dalla memoria
	free( dir );
	
	return 0;
}

/**
 * 
 * Cerca ricorsivamente tutte le risorse di nome $name e discendenti di
 * $dir, accodandole nella lsita list_found.
 * Gli elementi trovati saranno in ordine di path.
 * Ritorna 0 in caso tutto ok.
 * 
 */
int fs_find_r( fs_dir *dir, char *name, dll_tag *list_found ){
	
	rbt_iterator *sub_dir_index;
	fs_dir *sub_dir_ptr;
	fs_file *sub_file_ptr;
	int sub_file_founded, sub_file_pushed;
	
	sub_file_founded = 0;
	
	// check dati
	if( dir == NULL || name == NULL || list_found == NULL ){
		return -1;
	}
	
	// se esiste un file che è una delle risorse cercate
	sub_file_ptr = fs_get_sub_file( dir, name );
	if( sub_file_ptr != NULL ){
		// mi salvo che dovrò inserirlo, prima o poi, nella lista delle
		// risorse trovate, nell'ordine giusto
		sub_file_founded = 1;
		sub_file_pushed = 0;
	}
	
	//  itero sotto dir, tramite indice
	sub_dir_index = rbt_begin( dir->sub_dirs );
	while( sub_dir_index != NULL ){
		
		// passo dall'indice alla vera sotto dir
		sub_dir_ptr = ((rbt_node *)sub_dir_index)->val;
		
		// se la sub dir che sto analizzando seguirebbe il file valido che ho trovato
		if( sub_file_founded == 1 && sub_file_pushed == 0 && strcmp( sub_dir_ptr->name, sub_file_ptr->name ) > 0 ){  // strcmp() alla fine della catena di confronti perchè dovrebbe essere il confronto più lento
			// prima di procedere, inserisco il file
			dll_insert_end( list_found, sub_file_ptr, DLL_NODE_FILE );
			sub_file_pushed = 1;
		}
		
		// richiamo ricorsivamente su di essa
		fs_find_r( sub_dir_ptr, name, list_found );
		
		// se la dir è una delle risorse cercate
		if( sub_file_founded == 0 && strcmp( sub_dir_ptr->name, name ) == 0 ){  // non possono esserci file e dir con lo stesso nome
			dll_insert_end( list_found, sub_dir_ptr, DLL_NODE_DIR );
		}
		
		// passo alla sotto dir successiva
		sub_dir_index = rbt_next( dir->sub_dirs, sub_dir_index );
	}
	
	// se dopo aver analizzato tutte le sotto dir non ho ancora inserito il file
	if( sub_file_founded == 1 && sub_file_pushed == 0 ){
		// lo faccio
		dll_insert_end( list_found, sub_file_ptr, DLL_NODE_FILE );
		sub_file_pushed = 1;
	}
	
	return 0;
}

/**
 * 
 * Cerca tra tutti i discendenti di $dir le risorse di nome $name e ne
 * inserisce un ptr in una lista di cui ritorna il ptr al handle.
 * Ritorna NULL solo in caso di errore interno, non in caso di elementi
 * non trovati.
 * 
 */
dll_handle fs_find( fs_dir *dir, char *name ){
	
	dll_handle list_found = dll_new();
	
	// check dati
	if( dir == NULL || name == NULL ){
		return NULL;
	}
	
	return fs_find_r( dir, name, list_found ) == 0 ? list_found : NULL;  // torna null solo in caso di errore nella funz ricorsiva
}

//----------------------------------------------------------------------

/* implementazione RB Tree basata su: http://epaperpress.com/sortsearch/txt/rbtr.txt */

/**
 * 
 * Crea albero RB
 * 
 */
rbt_handle rbt_new( int (*rbt_compare)( void *a, void *b ) ){
	
	rbt_tag *rbt;
	
	rbt = (rbt_tag *)malloc( sizeof(rbt_tag) );
	if( rbt == NULL ){
		return NULL;
	}
	
	rbt->compare = rbt_compare;
	rbt->root = &rbt->sentinel;
	rbt->sentinel.left = &rbt->sentinel;
	rbt->sentinel.right = &rbt->sentinel;
	rbt->sentinel.parent = NULL;
	rbt->sentinel.color = BLACK;
	rbt->sentinel.key = NULL;
	rbt->sentinel.val = NULL;
	rbt->children_count = 0;
	
	return rbt;
}

/**
 * 
 * Cancella ricorsivamente albero, partendo dalle foglie
 * 
 */
void rbt_erase_tree( rbt_handle handle, rbt_node *node ){
	
	rbt_tag *rbt = handle;
	
	// mi fermo quando arrivo alle foglie
	if( node == &rbt->sentinel )
		return;
	
	// scendo nel percorso per cancellare dalle foglie
	rbt_erase_tree( handle, node->left );
	rbt_erase_tree( handle, node->right );
	
	free( node );
	rbt->children_count -= 1;
}

/**
 * 
 * Cancella albero
 * 
 */
void rbt_erase( rbt_handle handle ){
	
	rbt_tag *rbt = handle;
	
	// cancella ricorsivamente
	rbt_erase_tree( handle, rbt->root );
	
	free( rbt );
}

/**
 * 
 * Ruota nodo x a sinistra
 * 
 */
void rbt_rotate_left( rbt_tag *rbt, rbt_node *x ){
	
	rbt_node *y = x->right;
	
	// setto collegamento right di x
	x->right = y->left;
	if( y->left != &rbt->sentinel )
		y->left->parent = x;
	
	// setto parent per y
	if(y != &rbt->sentinel )
		y->parent = x->parent;
	if( x->parent ){
		if( x == x->parent->left )
			x->parent->left = y;
		else
			x->parent->right = y;
	} else{
		rbt->root = y;
	}
	
	// collego x e y
	y->left = x;
	if( x != &rbt->sentinel )
		x->parent = y;
	
}

/**
 * 
 * Ruota nodo x a destra
 * 
 */
void rbt_rotate_right(rbt_tag *rbt, rbt_node *x ){
	
	rbt_node *y = x->left;

	// setto collegamento left di x
	x->left = y->right;
	if( y->right != &rbt->sentinel )
		y->right->parent = x;
	
	// setto parent per y
	if( y != &rbt->sentinel )
		y->parent = x->parent;
	if( x->parent ){
		if( x == x->parent->right )
			x->parent->right = y;
		else
			x->parent->left = y;
	} else{
		rbt->root = y;
	}

	// collego x e y
	y->right = x;
	if( x != &rbt->sentinel )
		x->parent = y;
	
}

/**
 * 
 * Mantiene albero bilanciato dopo l'inserimento di x
 * 
 */
void rbt_insert_fixup( rbt_tag *rbt, rbt_node *x ){
	
	// finchè si ha problema di 2 R collegati direttamente
	while( x != rbt->root && x->parent->color == RED ){
		
		// parte sinistra
		if( x->parent == x->parent->parent->left ){
			
			rbt_node *y = x->parent->parent->right;
			
			// se uncle è R
			if( y->color == RED ){
				
				// ricoloro e controllo nuovamente alla successiva iterazione
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
				
			} else{
				
				// se x non è figlio sinistro
				if( x == x->parent->right ){
					// lo rendo tale
					x = x->parent;
					rbt_rotate_left( rbt, x );
				}
				
				// ricoloro e ruoto
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rbt_rotate_right( rbt, x->parent->parent );
				
			}
			
		} else{
			// parte destra (speculare a sopra)
			
			rbt_node *y = x->parent->parent->left;
			
			// se uncle è R
			if( y->color == RED ){
				
				// ricoloro e controllo nuovamente alla successiva iterazione
				x->parent->color = BLACK;
				y->color = BLACK;
				x->parent->parent->color = RED;
				x = x->parent->parent;
				
			} else{
				
				// se x non è figlio destro
				if( x == x->parent->left ){
					// lo rendo tale
					x = x->parent;
					rbt_rotate_right( rbt, x );
				}
				
				// ricoloro e ruoto
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rbt_rotate_left( rbt, x->parent->parent );
				
			}
			
		}
		
	}
	
	// ri-setto la radice come B
	rbt->root->color = BLACK;
}

/**
 * 
 * Crea e inserisce nell'albero un nodo con chiave e valori passati
 * 
 */
rbt_status rbt_insert( rbt_handle handle, void *key, void *val ){
	
	rbt_node *current, *parent, *x;
	rbt_tag *rbt = handle;
	
	// trovo posizione dove inserire il nodo
	current = rbt->root;
	parent = 0;
	while( current != &rbt->sentinel ){
		
		int result = rbt->compare( key, current->key );
		
		if( result == 0 )  // nomi delle sotto risorse devono essere univoci
			return RBT_STATUS_DUPLICATE_KEY;
		
		parent = current;
		current = ( result < 0) ? current->left : current->right;
	}

	// creo nuvo nodo
	x = malloc( sizeof( *x ) );
	if( x == 0 )
		return RBT_STATUS_MEM_EXHAUSTED;
	
	x->parent = parent;
	x->left = &rbt->sentinel;
	x->right = &rbt->sentinel;
	x->color = RED;
	x->key = key;
	x->val = val;
	
	// inserisco nodo nell'albero
	if( parent ){
		if( rbt->compare( key, parent->key ) < 0 )
			parent->left = x;
		else
			parent->right = x;
	} else{
		rbt->root = x;
	}
	rbt->children_count += 1;
	
	rbt_insert_fixup( rbt, x );
	
	return RBT_STATUS_OK;
}

/**
 * 
 * Mantiene albero bilanciato dopo la cancellazione del nodo x
 * 
 */
void rbt_delete_fixup( rbt_tag *rbt, rbt_node *x ){
	
	// finchè il nodo da cancellare è B
	while( x != rbt->root && x->color == BLACK ){
		
		// parte sinistra
		if( x == x->parent->left ){
			
			rbt_node *w = x->parent->right;
			
			if( w->color == RED ){
				
				w->color = BLACK;
				x->parent->color = RED;
				rbt_rotate_left( rbt, x->parent );
				w = x->parent->right;
				
			}
			
			if( w->left->color == BLACK && w->right->color == BLACK ){
				
				w->color = RED;
				x = x->parent;
				
			} else{
				
				if( w->right->color == BLACK ){
					
					w->left->color = BLACK;
					w->color = RED;
					rbt_rotate_right( rbt, w );
					w = x->parent->right;
					
				}
				
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rbt_rotate_left( rbt, x->parent );
				x = rbt->root;
				
			}
			
		} else{
			// parte destra (speculare a sopra)
			
			rbt_node *w = x->parent->left;
			
			if( w->color == RED ){
				
				w->color = BLACK;
				x->parent->color = RED;
				rbt_rotate_right( rbt, x->parent );
				w = x->parent->left;
				
			}
			
			if( w->right->color == BLACK && w->left->color == BLACK ){
				
				w->color = RED;
				x = x->parent;
			
			} else{
				
				if( w->left->color == BLACK ){
					
					w->right->color = BLACK;
					w->color = RED;
					rbt_rotate_left (rbt, w);
					w = x->parent->left;
					
				}
				
				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rbt_rotate_right (rbt, x->parent);
				x = rbt->root;
				
			}
			
		}
		
	}
	
	x->color = BLACK;
}

/**
 * 
 * Cancella dall'albero il nodo passato, eventualmente iterando per trovare il sostituto
 * 
 */
rbt_status rbt_delete( rbt_handle handle, rbt_iterator iterator ){
	
	rbt_node *x, *y;	// y nodo da eliminare, x il suo unico figio
	rbt_tag *rbt = handle;
	rbt_node *z = iterator;
	
	// se il nodo che vorremmo eliminare non ha figli
	if( z->left == &rbt->sentinel || z->right == &rbt->sentinel ){
		// lo eliminerò
		y = z;
	} else {
		// trovo il successore che lo sostituirà
		y = z->right;
		while( y->left != &rbt->sentinel )
			y = y->left;
	}
	
	// setto x
	if( y->left != &rbt->sentinel )
		x = y->left;
	else
		x = y->right;
	
	// rimuovo y dal percorso del padre
	x->parent = y->parent;
	if( y->parent )
		if( y == y->parent->left )
			y->parent->left = x;
		else
			y->parent->right = x;
	else
		rbt->root = x;
	
	// se il nodo da de-allocare è un altro
	if( y != z ){
		z->key = y->key;
		z->val = y->val;
	}
	
	// fixo solo se il nodo sbilancia l'albero
	if( y->color == BLACK )
		rbt_delete_fixup( rbt, x );
	
	free( y );
	rbt->children_count -= 1;
	
	return RBT_STATUS_OK;
}

/**
 * 
 * Ritorna il successivo al nodo passato
 * 
 */
rbt_iterator rbt_next( rbt_handle handle, rbt_iterator iterator ){
	
	rbt_tag *rbt = handle;
	rbt_node *i = iterator;
	
	// se ho sotto-nodi a destra
	if( i->right != &rbt->sentinel ){
		
		// è il minimo dei maggioranti
		for( i = i->right; i->left != &rbt->sentinel; i = i->left );
		
	} else {
		
		// torno su finchè sono a destra 
		rbt_node *p = i->parent;
		while( p && i == p->right ){
			i = p;
			p = p->parent;
		}
		
		i = p;
	}
	
	return i != &rbt->sentinel ? i : NULL;
}

/**
 * 
 * Riorna il primo nodo dell'albero
 * 
 */
rbt_iterator rbt_begin( rbt_handle handle ){
	
	rbt_tag *rbt = handle;
	rbt_node *i;
	
	for (i = rbt->root; i->left != &rbt->sentinel; i = i->left );
	
	return i != &rbt->sentinel ? i : NULL;
}

/**
 * 
 * Riorna l'ultimo nodo dell'albero
 * 
 */
rbt_iterator rbt_end( rbt_handle handle ){
	
	rbt_tag *rbt = handle;
	rbt_node *i;
	
	for (i = rbt->root; i->right != &rbt->sentinel; i = i->right );
	
	return i != &rbt->sentinel ? i : NULL;
}

rbt_node * rbt_find( rbt_handle handle, void *key ){
	
	rbt_tag *rbt = handle;
	
	rbt_node *current;
	current = rbt->root;
	
	while( current != &rbt->sentinel ){
		
		int result = rbt->compare( key, current->key );
		if( result == 0 )
			return current;
		
		current = (result < 0) ? current->left : current->right;
	}
	
	return NULL;
}

//----------------------------------------------------------------------

dll_handle dll_new(){
	
	dll_tag *list;
	
	list = (dll_tag *)malloc( sizeof(dll_tag) );
	if( list == NULL ){
		return NULL;
	}
	
	list->head = NULL;
	list->tail = NULL;
	list->nodes_count = 0;
	
	return list;
}

dll_status dll_insert_after( dll_handle handle, dll_node *node, void *val, dll_node_tag tag ){
	
	dll_tag *list = handle;
	dll_node *new_node;
	
	new_node = (dll_node *)malloc( sizeof(dll_node) );
	if( new_node == NULL ){
		return DLL_STATUS_MEM_EXHAUSTED;
	}
	
	new_node->val = val;
	new_node->tag = tag;
	
	new_node->prev = node;
	if( node->next == NULL ){
		new_node->next = NULL;
		list->tail = new_node;
	} else{
		new_node->next = node->next;
		node->next->prev = new_node;
	}
	node->next = new_node;
	
	list->nodes_count += 1;
	
	return DLL_STATUS_OK;
}

dll_status dll_insert_before( dll_handle handle, dll_node *node, void *val, dll_node_tag tag ){
	
	dll_tag *list = handle;
	dll_node *new_node;
	
	new_node = (dll_node *)malloc( sizeof(dll_node) );
	if( new_node == NULL ){
		return DLL_STATUS_MEM_EXHAUSTED;
	}
	
	new_node->val = val;
	new_node->tag = tag;
	
	new_node->next = node;
	if( node->prev == NULL ){
		new_node->prev = NULL;
		list->head = new_node;
	} else{
		new_node->prev = node->prev;
		node->prev->next = new_node;
	}
	node->prev = new_node;
	
	list->nodes_count += 1;
	
	return DLL_STATUS_OK;
}

dll_status dll_insert_beginning( dll_handle handle, void *val, dll_node_tag tag ){
	
	dll_tag *list = handle;
	dll_node *new_node;
	
	if( list->head == NULL ){
		
		new_node = (dll_node *)malloc( sizeof(dll_node) );
		if( new_node == NULL ){
			return DLL_STATUS_MEM_EXHAUSTED;
		}
		
		new_node->val = val;
		new_node->tag = tag;
		new_node->prev = NULL;
		new_node->next = NULL;
		
		list->head = new_node;
		list->tail = new_node;
		list->nodes_count = 1;
		
		return DLL_STATUS_OK;
		
	} else{
		return dll_insert_before( list, list->head, val, tag );
	}
}

dll_status dll_insert_end( dll_handle handle, void *val, dll_node_tag tag ){
	
	dll_tag *list = handle;
	
	if( list->tail == NULL ){
		return dll_insert_beginning( list, val, tag );
		
	} else{
		return dll_insert_after( list, list->tail, val, tag );
	}
}

void * dll_delete( dll_handle handle, dll_node *node ){
	
	dll_tag *list = handle;
	void *val = node->val;
	
	if( node->prev == NULL ){
		list->head = node->next;
	} else{
		node->prev->next = node->next;
	}
	
	if( node->next == NULL ){
		list->tail = node->prev;
	} else{
		node->next->prev = node->prev;
	}
	
	free( node );
	list->nodes_count -= 1;
	
	return val;
}

void * dll_delete_beginning( dll_handle handle ){
	return dll_delete( handle, ((dll_tag *)handle)->head );
}

void * dll_delete_end( dll_handle handle ){
	return dll_delete( handle, ((dll_tag *)handle)->tail );
}

void dll_erase( dll_handle handle ){
	
	dll_tag *list = handle;
	dll_node *node1, *node2;
	
	node1  = list->head;
	while( node1 != NULL ){
		node2 = node1->next;
		free( node1 );
		node1 = node2;
	}
	
	free( list );
}
