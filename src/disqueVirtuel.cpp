/**
 * \file disqueVirtuel.cpp
 * \brief Implémentation d'un disque virtuel.
 * \author Maxime Miville Deschenes, Philippe Vincent, Francis Boulianne
 * \version 0.1
 * \date  2021
 *
 *  Travail pratique numéro 3
 *
 */

#include "disqueVirtuel.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

//vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3
{

    DisqueVirtuel::DisqueVirtuel() {
        m_blockDisque.resize(N_BLOCK_ON_DISK);
    }

    DisqueVirtuel::~DisqueVirtuel() {
        // on clear le disque virtuel
        m_blockDisque.clear();
    }

	// Ajouter votre code ici !
	int DisqueVirtuel::bd_FormatDisk()
	{

		try
		{
			erase_virtual_disk();
			initialize_bitmap_on_block_two();
			create_inodes_on_disk();
			set_inode_from_one_to_nineteen_to_free();
			create_directory_inodes();

        }
		catch (...)
		{
			return 0;
		}

		return 1;
	}

	// on vide les diques de toutes les blocks qu'il contient
	void DisqueVirtuel::erase_virtual_disk()
	{
		m_blockDisque.clear();
	}

	// on initialise le liste de blocs libres (bitmap) sur le block 2
	void DisqueVirtuel::initialize_bitmap_on_block_two()
	{
		Block bitmap_block_free = Block(S_IFBL);
		bitmap_block_free.m_bitmap.resize(N_BLOCK_ON_DISK);

		// comme mentionné, on mets touts les blocks de 0 à 23 non libres
		// pourrait le mettre dans la classe block


		for (int i = 0; i <= 23; i++)
		{
			bitmap_block_free.m_bitmap[i] = false;
		}

		// on mets touts les autres blocks libres
		// pourrait le mettre dans la classe block

		for (int i = 24; i < 128; i++)
		{
			bitmap_block_free.m_bitmap[i] = true;
		}


		// on initialie le bloc 2 par le bloc de blocs  libres
		m_blockDisque[FREE_BLOCK_BITMAP] = bitmap_block_free;
	}

	// on créer les inodes libres sur le disque
	void DisqueVirtuel::create_inodes_on_disk()
	{
		Block inodes_block = Block(S_IFIN);

		// créer les i-nodes dans les blocs de 4 à 23;
		for (int i = 6; i <= 23; i++)
		{
			m_blockDisque[i] = inodes_block;
			m_blockDisque[i].m_inode = new iNode(i - BASE_BLOCK_INODE, S_IFREG, 0, 0, 0);
		}
	}

	// on créer le inodes racines
	void DisqueVirtuel::create_directory_inodes()
	{
		Block repertory_inodes = Block(S_IFIN);
		size_t first_free_block = find_free_block();

		m_blockDisque[ROOT_INODE + BASE_BLOCK_INODE].m_inode = new iNode(ROOT_INODE, S_IFDIR, 1, 28, first_free_block);

		Block block_repertory = Block(S_IFDE);
		m_blockDisque[first_free_block] = block_repertory;

        change_inode_disponibility(ROOT_INODE, false);
        change_block_disponibility(first_free_block, false);
    }

	void DisqueVirtuel::set_inode_from_one_to_nineteen_to_free()
	{

		Block bitmap_inodes_free = Block(S_IFIL);

        bitmap_inodes_free.m_bitmap[0] = false;

        // comme mentionnée, on mets toutes les blocks inodes de 1 à 19 libres
		for (int i = 1; i <= 19; i++)
		{
			bitmap_inodes_free.m_bitmap[i] = true;
		}


		// on initialie le bloc 3 par le bloc d'i-nodes libres
		m_blockDisque[FREE_INODE_BITMAP] = bitmap_inodes_free;
	}



	int DisqueVirtuel::bd_create(const std::string &p_FileName)
	{
        // on convertie le path en un vecteur de string
        std::vector<std::string> directories_with_file = convert_string_to_vector(p_FileName);
        size_t numbers_of_directories = directories_with_file.size();

        // si le vecteur de string ne contient aucun path, on retourne 0
        if (numbers_of_directories == 0) {
            return 0;
        }

        // on va voir si le path existe
        size_t index_of_full_path = does_path_exist(directories_with_file);

        //si le path complet existe, on ne fait rien car on ne veux pas créer de fichier
        if (index_of_full_path != -1) {
            return 0;
        }

        std::vector<std::string> directories_path = directories_with_file;

        // on enleve le nom du fichier dans le vecteur
        directories_path.pop_back();

        size_t index_parent_of_path = does_path_exist(directories_path);

        // si le path parent n'existe pas, on retourne tout de suite sans ajouter le fichier
        if (index_parent_of_path == -1) {
            return 0;
        }

        //trouver le premier iNode libre
        size_t first_free_node = find_free_inode();

        //trouver le premier  block libre
        size_t first_block_free = find_free_block();

        //Creer nouveau block de type iNode pour le fichier
        Block new_block_i_node = Block(S_IFIN);

        // on met le nouveau i-node dans le disque
        m_blockDisque[first_free_node + BASE_BLOCK_INODE] = new_block_i_node;
        m_blockDisque[first_free_node + BASE_BLOCK_INODE].m_inode = new iNode(first_free_node, S_IFREG, 1, 0, first_block_free);

        // on créer un block pour le fichier
        Block block = Block();

        // on assigne le block à l'index du premier block libre
        m_blockDisque[first_block_free] = block;

        // on va chercher le numero de block du parent
        size_t index_block_parent = m_blockDisque[index_parent_of_path + BASE_BLOCK_INODE].m_inode->st_block;

        // on ajoute le nouveau fichier au parents
        m_blockDisque[index_block_parent].m_dirEntry.push_back(new dirEntry(first_free_node, directories_with_file[numbers_of_directories - 1]));

        //Changer la disponibilite de iNode
        change_inode_disponibility(first_free_node, false);
        change_block_disponibility(first_block_free, false);

        return 1;
	}

	// convertie le path en un vecteur de string
	std::vector<std::string> DisqueVirtuel::convert_string_to_vector(const std::string &directory_name)
	{
		std::vector<std::string> vector_of_string;

		std::string str = directory_name;
		const char delim = '/';
		size_t start;
		size_t end = 0;

		while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
		{
			end = str.find(delim, start);
			vector_of_string.push_back(str.substr(start, end - start));
		}

		return vector_of_string;
	}


	std::string DisqueVirtuel::get_directory_name(const std::string &file)
	{
		size_t last_slash = file.find_last_of("/");
		std::string directory = file.substr(0, last_slash);
		return directory;
	}

	std::string DisqueVirtuel::get_file_name(const std::string &file)
	{
		size_t last_slash = file.find_last_of("/");
		std::string name_of_file = file.substr(last_slash + 1, last_slash + 1);
		return name_of_file;
	}


	bool DisqueVirtuel::does_file_exists(const std::string &p_file_name)
	{
		for (int i = 0; i < m_blockDisque.size(); i++)
		{
			for (int j = 0; j < m_blockDisque[i].m_dirEntry.size(); j++)
			{
				if (m_blockDisque[i].m_dirEntry[j]->m_filename.compare(p_file_name) == 0)
				{
					size_t numero_inode = m_blockDisque[i].m_dirEntry[j]->m_iNode;
					if (m_blockDisque[numero_inode].m_inode->st_mode == S_IFREG)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool DisqueVirtuel::does_directory_exists(const std::string &p_dir_name)
	{
		for (int i = 0; i < m_blockDisque.size(); i++)
		{
			for (int j = 0; j < m_blockDisque[i].m_dirEntry.size(); j++)
			{
				if (m_blockDisque[i].m_dirEntry[j]->m_filename.compare(p_dir_name) == 0)
				{
					size_t numero_inode = m_blockDisque[i].m_dirEntry[j]->m_iNode;
					if (m_blockDisque[numero_inode].m_inode->st_mode == S_IFDIR)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	size_t DisqueVirtuel::return_index_of_directory(const std::string &p_dir_name)
	{
		for (int i = 0; i < m_blockDisque.size(); i++)
		{
			for (int j = 0; j < m_blockDisque[i].m_dirEntry.size(); j++)
			{
				if (m_blockDisque[i].m_dirEntry[j]->m_filename.compare(p_dir_name) == 0)
				{
					size_t numero_inode = m_blockDisque[i].m_dirEntry[j]->m_iNode;
					if (m_blockDisque[numero_inode].m_inode->st_mode == S_IFDIR)
					{
						return numero_inode;
					}
				}
			}
		}

		return 0;
	}

	int DisqueVirtuel::bd_mkdir(const std::string &p_DirName)
	{
		std::vector<std::string> directories = convert_string_to_vector(p_DirName);
		size_t numbers_of_directories = directories.size();

		if (numbers_of_directories == 0) {
		    return 0;
		}

        size_t new_path = does_path_exist(directories);

        if (new_path != -1) {
            return 0;
        }

        std::vector<std::string> directories_without_new_directory = directories;
        directories_without_new_directory.pop_back();

		size_t index_of_parent_inodes = does_path_exist(directories_without_new_directory);

		if (index_of_parent_inodes == -1) {
		    return 0;
		}

		size_t index_of_parents_block = m_blockDisque[index_of_parent_inodes + BASE_BLOCK_INODE].m_inode->st_block;

		//trouver l'index du premier iNode disponible
		size_t inode_index = find_free_inode();
		//trouver l'index du premier block disponible
		size_t block_index = find_free_block();

		//créer un nouvel iNode
		Block block_with_inode = Block(S_IFIN);
        m_blockDisque[inode_index + BASE_BLOCK_INODE] = block_with_inode;
        m_blockDisque[inode_index + BASE_BLOCK_INODE].m_inode = new iNode(inode_index, S_IFDIR, 2, 28, block_index);
		// on ajoute le repo aux parents
		m_blockDisque[index_of_parents_block].m_dirEntry.push_back(new dirEntry(inode_index, directories[numbers_of_directories - 1]));

		// on ajoute données à l'ione du parents
		m_blockDisque[index_of_parent_inodes + BASE_BLOCK_INODE].m_inode->st_nlink++;
		m_blockDisque[index_of_parent_inodes + BASE_BLOCK_INODE].m_inode->st_size += 28;

		//Ajouter les directory ".." et "." au nouveau directory
		Block block = Block(S_IFDE);
        m_blockDisque[block_index] = block;
        m_blockDisque[block_index].m_dirEntry.push_back(new dirEntry(index_of_parent_inodes, ".."));
        m_blockDisque[block_index].m_dirEntry.push_back(new dirEntry(inode_index, "."));

		//changer disponibilité du iNode dans la bitmap
		change_inode_disponibility(inode_index, false);
		change_block_disponibility(block_index, false);
		return 1;
	}

	// pour trouver le premier inodes de libres
	size_t DisqueVirtuel::find_free_inode()
	{
		for (size_t i = 0; i < m_blockDisque[FREE_INODE_BITMAP].m_bitmap.size(); i++)
		{
			if (m_blockDisque[FREE_INODE_BITMAP].m_bitmap[i])
			{
				return i;
			}
		}
		return 0;
	}

	// pour trouver le premier block de libres dans le bitmap
	size_t DisqueVirtuel::find_free_block()
	{
		for (size_t i = 0; i < m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap.size(); i++)
		{
			if (m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i])
			{
				return i;
			}
		}
		return 0;
	}

	// changer la disponibilités des inodes dans le bitmap
	void DisqueVirtuel::change_inode_disponibility(int iNode_index, bool set_to)
	{
            m_blockDisque[FREE_INODE_BITMAP].m_bitmap[iNode_index] = set_to;
	}

	// changer la disponibilités des blocks dans le bitmap
	void DisqueVirtuel::change_block_disponibility(int block_index, bool set_to)
	{
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[block_index] = set_to;
	}


    size_t DisqueVirtuel::does_path_exist(std::vector<std::string> &directories) {

        if (directories.size() == 0) {
            return ROOT_INODE;
        }

        size_t last_index_of_repo = 0;

        for (int i = 0; i < directories.size(); i++) {
            bool does_repo_exist = false;

            if (i == 0) {
               int root_block =  m_blockDisque[ROOT_INODE + BASE_BLOCK_INODE].m_inode->st_block;
               for (int j = 0; j < m_blockDisque[root_block].m_dirEntry.size(); j++) {
                   if (m_blockDisque[root_block].m_dirEntry[j]->m_filename.compare(directories[i]) == 0) {
                       last_index_of_repo = m_blockDisque[root_block].m_dirEntry[j]->m_iNode;
                       does_repo_exist = true;
                   }
               }
            } else {
               size_t root_block = m_blockDisque[last_index_of_repo + BASE_BLOCK_INODE].m_inode->st_block;
               for (int p = 0; p < m_blockDisque[root_block].m_dirEntry.size(); p++) {
                   if (m_blockDisque[root_block].m_dirEntry[p]->m_filename.compare(directories[i]) == 0) {
                       last_index_of_repo = m_blockDisque[root_block].m_dirEntry[p]->m_iNode;
                       does_repo_exist = true;
                   }
               }
            }
            if (!does_repo_exist) {
                return -1;
            }
        }
        return last_index_of_repo;
    }

	

	std::string DisqueVirtuel::bd_ls(const std::string &p_DirLocation)
	{
        std::ostringstream ls_stream;
		std::vector<std::string> directories = convert_string_to_vector(p_DirLocation);
        size_t number_of_index_inodes = does_path_exist(directories);

        if (number_of_index_inodes == - 1) {
            return ls_stream.str();
        }


        size_t number_of_block_index = m_blockDisque[number_of_index_inodes + BASE_BLOCK_INODE].m_inode->st_block;
        Block block_with_all_files_and_directory = m_blockDisque[number_of_block_index];

			for (int i = 0; i < block_with_all_files_and_directory.m_dirEntry.size(); i++)
			{
                    std::string name = block_with_all_files_and_directory.m_dirEntry[i]->m_filename;
                    size_t numero_inodes = block_with_all_files_and_directory.m_dirEntry[i]->m_iNode;

                    size_t mode = m_blockDisque[numero_inodes + BASE_BLOCK_INODE].m_inode->st_mode;
                    size_t number_of_link = m_blockDisque[numero_inodes + BASE_BLOCK_INODE].m_inode->st_nlink;
                    size_t size_of_file_or_dir = m_blockDisque[numero_inodes + BASE_BLOCK_INODE].m_inode->st_size;

                    std::string champ = "";

                    if (mode == S_IFREG)
                    {
                        champ = "-";
                    }
                    else if (mode == S_IFDIR)
                    {
                        champ = "d";
                    }
                    else
                    {
                        champ = "X";
                    }
                    ls_stream << champ << "      " << name << "   Size:   " << size_of_file_or_dir << "   inode:    " << numero_inodes << "   nlink:   " << number_of_link << std::endl;
			    }

		return ls_stream.str();
	}


	// trouver le numero du inodes dans la liste des directories
	size_t DisqueVirtuel::get_numbers_of_inodes(const std::string &name) {
		for (int i = 0; i < m_blockDisque.size(); i++)
		{
			for (int j = 0; j < m_blockDisque[i].m_dirEntry.size(); j++)
			{
				if (m_blockDisque[i].m_dirEntry[j]->m_filename.compare(name) == 0)
				{
					return m_blockDisque[i].m_dirEntry[j]->m_iNode;
				}
			}
		}
		return -1;
	}

	// trouver si c'est un fichier ou un repertoire
	size_t DisqueVirtuel::is_name_a_directory_or_an_file(std::string &name) {
		size_t index_iones = get_numbers_of_inodes(name);

		size_t file_or_directory = m_blockDisque[index_iones].m_inode->st_mode;

		return file_or_directory;
	}



	int DisqueVirtuel::bd_rm(const std::string &p_Filename)
	{

        //convertir le path en un vecteur de string
        std::vector<std::string> directories = convert_string_to_vector(p_Filename);
        size_t numbers_of_directories = directories.size();

        if (numbers_of_directories == 0) {
            return 0;
        }

        //aller chercher l'iondes de l'index du fichier à suppriemr
        size_t inodes_index_of_file = does_path_exist(directories);

        //si le fichier n'existe pas, on ne fait rien
        if (inodes_index_of_file == -1) {
            return 0;
        }

        // on va chercher l'index de l'inodes du parents
        std::vector<std::string> parent_directories = directories;
        parent_directories.pop_back();
        size_t inodes_index_of_parents = does_path_exist(parent_directories);

        if (inodes_index_of_parents == -1) {
            return 0;
        }

        // on va chercher le bloc du fichier ou du repository
        size_t index_of_block = m_blockDisque[inodes_index_of_file + BASE_BLOCK_INODE].m_inode->st_block;
        size_t mode_of_block = m_blockDisque[inodes_index_of_file + BASE_BLOCK_INODE].m_inode->st_mode;


        // si c'est un repertoire, s'assurer qu'il soit pas vide, si pas vide, on arrête
        if (mode_of_block == S_IFDIR) {
            if (m_blockDisque[index_of_block].m_dirEntry.size() > 2) {
                return 0;
            } else {
                m_blockDisque[index_of_block].m_dirEntry.clear();
            }
        }

        //on va chercher le block du parents
        size_t block_parent = m_blockDisque[inodes_index_of_parents + BASE_BLOCK_INODE].m_inode->st_block;

        // on détruit l'entré dans le dirEntry du parents
        for (int i = 0; i < m_blockDisque[block_parent].m_dirEntry.size(); i++) {
            if (m_blockDisque[block_parent].m_dirEntry[i]->m_filename.compare(directories[numbers_of_directories - 1]) == 0) {
                // on enlève le repertoire et on décrement st_nlink
                m_blockDisque[block_parent].m_dirEntry.erase(m_blockDisque[block_parent].m_dirEntry.begin() + i);
                m_blockDisque[inodes_index_of_parents + BASE_BLOCK_INODE].m_inode->st_nlink--;
            }
        }

        // on décremente le nombre de links pour l'inodes
        m_blockDisque[inodes_index_of_file + BASE_BLOCK_INODE].m_inode->st_nlink--;
        size_t nb_links = m_blockDisque[inodes_index_of_file + BASE_BLOCK_INODE].m_inode->st_nlink;


        // si l'inodes est de 0 ou plus petit, on libère les blocks
        if (nb_links <= 0) {
            change_block_disponibility(index_of_block, false);
            change_inode_disponibility(inodes_index_of_file, false);
        }

        return 1;
	}

	}