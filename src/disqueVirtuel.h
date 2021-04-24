/**
 * \file disqueVirtuel.h
 * \brief Gestion d'un disque virtuel.
 * \author GLO-2001
 * \version 0.1
 * \date  2021
 *
 *  Travail pratique numéro 3
 *
 */

#include "block.h"

#ifndef _DISQUEVIRTUEL__H
#define _DISQUEVIRTUEL__H

namespace TP3
{

#define N_INODE_ON_DISK 20	// nombre maximal d'i-nodes (donc de fichiers) sur votre disque
#define N_BLOCK_ON_DISK 128 // nombre de blocs sur le disque au complet
#define FREE_BLOCK_BITMAP 2 // numero du bloc contenant le bitmap des block libres
#define FREE_INODE_BITMAP 3 // numero du bloc contenant le bitmap des i-nodes libres
#define BASE_BLOCK_INODE 4	// bloc de depart ou les i-nodes sont stockes sur disque
#define ROOT_INODE 1		// numero du i-node correspondant au repertoire racine

	class DisqueVirtuel
	{
	public:
		DisqueVirtuel();
		~DisqueVirtuel();

		// Méthodes demandées
		int bd_FormatDisk();
		std::string bd_ls(const std::string &p_DirLocation);
		int bd_mkdir(const std::string &p_DirName);
		int bd_create(const std::string &p_FileName);
		int bd_rm(const std::string &p_Filename);
		bool does_file_exists(const std::string &p_Filename);
		bool does_directory_exists(const std::string &p_Dirname);
		bool is_file_mode_ok(const std::string &p_Filename, int mode);
		size_t find_free_inode();
		size_t find_free_block();
		size_t find_directory_index(const std::string &dir);
		void change_inode_disponibility(int iNode_index, bool set_to);
		void change_block_disponibility(int block_index, bool set_to);
		// pour méthode bd_FormatDisk()
		void erase_virtual_disk();
		void initialize_bitmap_on_block_two();
		void create_inodes_on_disk();
		void set_inode_from_one_to_nineteen_to_free();
		void create_directory_inodes();

		// pour méthode bd_create
		std::string get_file_name(const std::string &file);
		std::string get_directory_name(const std::string &file);
		size_t return_index_of_directory(const std::string &p_dir_name);

		// utils
		bool does_new_directory_is_valid(std::vector<std::string> &directories);
		std::vector<std::string> convert_string_to_vector(const std::string &directory_name);
		size_t does_directory_or_files_exist(const std::string &name);
		size_t return_index_of_inodes(const std::string &p_dir_name);

		// utils
		size_t does_path_exist(std::vector<std::string> &directories);

		size_t is_name_a_directory_or_an_file(std::string &name);
		size_t get_numbers_of_inodes(const std::string &name);

	private:
		// Il est interdit de modifier ce modèle d'implémentation (i.e le type de m_blockDisque)!
		std::vector<Block> m_blockDisque; // Un vecteur de blocs représentant le disque virtuel

		// Vous pouvez ajouter ici des méthodes privées
	};

} //Fin du namespace

#endif