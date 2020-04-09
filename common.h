// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#include <cstddef>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <memory>
#include <limits>
#include <algorithm>
#include <numeric>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "seal/seal.h"

using namespace std;
using namespace seal;

struct thread_para{
    int fd;
    shared_ptr<SEALContext> context;
};

#define clustar_flag  "\
+-------------------------------------------------------------------------------+\n\
|                                                                               |\n\
|     |||||   |         |       |  ||||||||  |||||||||       |        ||||||    |\n\
|    |        |         |       |  |             |          | |       |     |   |\n\
|   |         |         |       |  |             |         |   |      |     |   |\n\
|   |         |         |       |  ||||||||      |        |||||||     ||||||    |\n\
|   |         |         |       |         |      |       |       |    |   |     |\n\
|    |        |         |       |         |      |      |         |   |    |    |\n\
|     |||||   ||||||||  |||||||||  ||||||||      |     |           |  |     |   |\n\
|                                                                               |\n\
+-------------------------------------------------------------------------------+\n"

inline void add_plain_helper(int op, shared_ptr<SEALContext> context);
inline void mul_helper(int op, shared_ptr<SEALContext> context);
inline void square_helper(int op, shared_ptr<SEALContext> context);
void calc_bfv_basic();
int muti_core_runner();

/*
Helper function: Prints the name of the example in a fancy banner.
*/
inline void print_example_banner(std::string title)
{
    if (!title.empty())
    {
        std::size_t title_length = title.length();
        std::size_t banner_length = title_length + 2 * 10;
        std::string banner_top = "+" + std::string(banner_length - 2, '-') + "+";
        std::string banner_middle =
            "|" + std::string(9, ' ') + title + std::string(9, ' ') + "|";

        std::cout << std::endl
            << banner_top << std::endl
            << banner_middle << std::endl
            << banner_top << std::endl;
    }
}

/*
Helper function: Prints the parameters in a SEALContext.
*/
inline void print_parameters(std::shared_ptr<seal::SEALContext> context){
    // Verify parameters
    if (!context)
    {
        throw std::invalid_argument("context is not set");
    }
    auto &context_data = *context->key_context_data();

    /*
    Which scheme are we using?
    */
    std::string scheme_name;
    switch (context_data.parms().scheme())
    {
    case seal::scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case seal::scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        throw std::invalid_argument("unsupported scheme");
    }
    std::cout << "/" << std::endl;
    std::cout << "| Encryption parameters :" << std::endl;
    std::cout << "|   scheme: " << scheme_name << std::endl;
    std::cout << "|   poly_modulus_degree: " <<
        context_data.parms().poly_modulus_degree() << std::endl;

    /*
    Print the size of the true (product) coefficient modulus.
    */
    std::cout << "|   coeff_modulus size: ";
    std::cout << context_data.total_coeff_modulus_bit_count() << " (";
    auto coeff_modulus = context_data.parms().coeff_modulus();
    std::size_t coeff_mod_count = coeff_modulus.size();
    for (std::size_t i = 0; i < coeff_mod_count - 1; i++){
        std::cout << coeff_modulus[i].bit_count() << " + ";
    }
    std::cout << coeff_modulus.back().bit_count();
    std::cout << ") bits" << std::endl;

    /*
    For the BFV scheme print the plain_modulus parameter.
    */
    if (context_data.parms().scheme() == seal::scheme_type::BFV)
    {
        std::cout << "|   plain_modulus: " << context_data.
            parms().plain_modulus().value() << std::endl;
    }

    std::cout << "\\" << std::endl;
}


inline void print_result(int op, std::shared_ptr<seal::SEALContext> context, int num1, int num2, size_t size_en, size_t noise_en, string polynomial, size_t res){
    auto &context_data = *context->key_context_data();
    std::string scheme_name;
    std::string op_name;
    switch(op){
        case 1: op_name = "Add"; break;
        case 2: op_name = "Mul"; break;
        case 3: op_name = "Square"; break;
        default: printf("Unsupported Compute Mode!!!\n"); return;
    }
    switch (context_data.parms().scheme())
    {
    case seal::scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case seal::scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        throw std::invalid_argument("unsupported scheme");
    }
    fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "|                                      TEST RESULT                                       |\n");
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Task Type                 | %-58s |\n", op_name.c_str());
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Number 1                  | %-58d |\n", num1);
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Number 2                  | %-58d |\n", num2);
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "|                                                                                        |\n");
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Encryption Parameters                                                                  |\n");
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Scheme                      | %-56s |\n", scheme_name.c_str());
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Poly_modulus_degree         | %-56d |\n", context_data.parms().poly_modulus_degree());
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Coeff_modulus size          | %-56d |\n", context_data.total_coeff_modulus_bit_count());
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "| Plain_modulus               | %-56d |\n", context_data.parms().plain_modulus().value());
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
    fprintf(stdout, "|                                                                                        |\n");
    fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
    fprintf(stdout, "| Result                                                                                 |\n");
    fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
    fprintf(stdout, "| Polynomial                  | %-56s |\n", polynomial.c_str());
    fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
    fprintf(stdout, "| Size_encrypt                | %-56d |\n", size_en);
    fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
    fprintf(stdout, "| Noise_budget                | %-56d |\n", noise_en);
	fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
    fprintf(stdout, "| Output                      | %-56d |\n", res);
    fprintf(stdout, "+----------------------------------------------------------------------------------------+\n");
	fprintf(stdout, "\n");
}