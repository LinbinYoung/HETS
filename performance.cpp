// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "common.h"

#define MAXS 18000 //running for 3 minutes

void* bfv_performance(void *th_para){
    /* Get the current timestamp
    */
    struct thread_para *para = (struct thread_para *) th_para;
    string path = "../clustar/record/log";
    path = path + to_string(para->fd);
    ofstream LogVVV(path);
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff;
    auto context = para->context;
    auto &parms = context->first_context_data()->parms();
    auto &plain_modulus = parms.plain_modulus();
    size_t poly_modulus_degree = parms.poly_modulus_degree();
    /* Generating secret/public keys
    */
    time_start = chrono::high_resolution_clock::now();
    KeyGenerator keygen(context);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    LogVVV << "Generate Keys Done: " << time_diff.count() << " microseconds" << endl;
    auto secret_key = keygen.secret_key();
    auto public_key = keygen.public_key();
    RelinKeys relin_keys;
    GaloisKeys gal_keys;
    if (context->using_keyswitching()){
        /* Generate relinearization keys
        */
        time_start = chrono::high_resolution_clock::now();
        relin_keys = keygen.relin_keys();
        time_end = chrono::high_resolution_clock::now();
        time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
        LogVVV << "Generate Relinearization Keys Done: " << time_diff.count() << " microseconds" << endl;
        if (!context->key_context_data()->qualifiers().using_batching){
            LogVVV << "Given encryption parameters do not support batching." << endl;
            return NULL;
        }
        /* Generating Galois keys
        */
        time_start = chrono::high_resolution_clock::now();
        gal_keys = keygen.galois_keys();
        time_end = chrono::high_resolution_clock::now();
        time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
        LogVVV << "Generating Galois Key Done: " << time_diff.count() << " microseconds" << endl;
    }

    Encryptor encryptor(context, public_key);
    Decryptor decryptor(context, secret_key);
    Evaluator evaluator(context);
    BatchEncoder batch_encoder(context);
    IntegerEncoder encoder(context);

    /* These will hold the total times used by each operation.
    */
    chrono::microseconds time_batch_sum(0);
    chrono::microseconds time_unbatch_sum(0);
    chrono::microseconds time_encrypt_sum(0);
    chrono::microseconds time_decrypt_sum(0);
    chrono::microseconds time_add_sum(0);
    chrono::microseconds time_multiply_sum(0);
    chrono::microseconds time_multiply_plain_sum(0);
    chrono::microseconds time_square_sum(0);
    chrono::microseconds time_relinearize_sum(0);
    chrono::microseconds time_rotate_rows_one_step_sum(0);
    chrono::microseconds time_rotate_rows_random_sum(0);
    chrono::microseconds time_rotate_columns_sum(0);

    /* Populate a vector of values to batch.
    */
    size_t slot_count = batch_encoder.slot_count();
    vector<uint64_t> pod_vector;
    for (size_t i = 0; i < slot_count; i++){
        pod_vector.push_back(1000000 % plain_modulus.value());
    }
    long long count = 0;
    chrono::high_resolution_clock::time_point time_start_g, time_end_g;
    chrono::microseconds time_diff_g;
    time_start_g = chrono::high_resolution_clock::now();
    while (1){
        /*
        [Batching]
        There is nothing unusual here. We batch our random plaintext matrix
        into the polynomial. Note how the plaintext we create is of the exactly
        right size so unnecessary reallocations are avoided.
        */
        Plaintext plain(parms.poly_modulus_degree(), 0);
        time_start = chrono::high_resolution_clock::now();
        batch_encoder.encode(pod_vector, plain);
        time_end = chrono::high_resolution_clock::now();
        time_batch_sum += chrono::duration_cast<chrono::microseconds>(time_end - time_start);
        /*
        [Unbatching]
        We unbatch what we just batched.
        */
        vector<uint64_t> pod_vector2(slot_count);
        time_start = chrono::high_resolution_clock::now();
        batch_encoder.decode(plain, pod_vector2);
        time_end = chrono::high_resolution_clock::now();
        time_unbatch_sum += chrono::duration_cast<chrono::microseconds>(time_end - time_start);
        if (pod_vector2 != pod_vector)
        {
            throw runtime_error("Batch/unbatch failed. Something is wrong.");
        }

        /*
        [Encryption]
        We make sure our ciphertext is already allocated and large enough
        to hold the encryption with these encryption parameters. We encrypt
        our random batched matrix here.
        */
        Ciphertext encrypted(context);
        time_start = chrono::high_resolution_clock::now();
        encryptor.encrypt(plain, encrypted);
        time_end = chrono::high_resolution_clock::now();
        time_encrypt_sum += chrono::duration_cast<chrono::microseconds>(time_end - time_start);

        /*
        [Decryption]
        We decrypt what we just encrypted.
        */
        Plaintext plain2(poly_modulus_degree, 0);
        time_start = chrono::high_resolution_clock::now();
        decryptor.decrypt(encrypted, plain2);
        time_end = chrono::high_resolution_clock::now();
        time_decrypt_sum += chrono::duration_cast<chrono::microseconds>(time_end - time_start);
        if (plain2 != plain){
            throw runtime_error("Encrypt/decrypt failed. Something is wrong.");
        }

        /*
        [Add]
        We create two ciphertexts and perform a few additions with them.
        */
        Ciphertext encrypted1(context);
        encryptor.encrypt(encoder.encode(static_cast<uint64_t>(100)), encrypted1);
        Ciphertext encrypted2(context);
        encryptor.encrypt(encoder.encode(static_cast<uint64_t>(100 + 1)), encrypted2);
        time_start = chrono::high_resolution_clock::now();
        evaluator.add_inplace(encrypted1, encrypted1);
        // evaluator.add_inplace(encrypted2, encrypted2);
        // evaluator.add_inplace(encrypted1, encrypted2);
        time_end = chrono::high_resolution_clock::now();
        time_add_sum += chrono::duration_cast<chrono::microseconds>(time_end - time_start);

        /*
        [Multiply]
        We multiply two ciphertexts. Since the size of the result will be 3,
        and will overwrite the first argument, we reserve first enough memory
        to avoid reallocating during multiplication.
        */ 
        encrypted1.reserve(3);
        time_start = chrono::high_resolution_clock::now();
        evaluator.multiply_inplace(encrypted1, encrypted2);
        time_end = chrono::high_resolution_clock::now();
        time_multiply_sum += chrono::duration_cast<chrono::microseconds>(time_end - time_start);

        /*
        [Multiply Plain]
        We multiply a ciphertext with a random plaintext. Recall that
        multiply_plain does not change the size of the ciphertext so we use
        encrypted2 here.
        */
        time_start = chrono::high_resolution_clock::now();
        evaluator.multiply_plain_inplace(encrypted2, plain);
        time_end = chrono::high_resolution_clock::now();
        time_multiply_plain_sum += chrono::duration_cast<chrono::microseconds>(time_end - time_start);

        /*
        [Square]
        We continue to use encrypted2. Now we square it; this should be
        faster than generic homomorphic multiplication.
        */
        time_start = chrono::high_resolution_clock::now();
        evaluator.square_inplace(encrypted2);
        time_end = chrono::high_resolution_clock::now();
        time_square_sum += chrono::duration_cast<
            chrono::microseconds>(time_end - time_start);

        if (context->using_keyswitching())
        {
            /*
            [Relinearize]
            Time to get back to encrypted1. We now relinearize it back
            to size 2. Since the allocation is currently big enough to
            contain a ciphertext of size 3, no costly reallocations are
            needed in the process.
            */
            time_start = chrono::high_resolution_clock::now();
            evaluator.relinearize_inplace(encrypted1, relin_keys);
            time_end = chrono::high_resolution_clock::now();
            time_relinearize_sum += chrono::duration_cast<
                chrono::microseconds>(time_end - time_start);

            /*
            [Rotate Rows One Step]
            We rotate matrix rows by one step left and measure the time.
            */
            time_start = chrono::high_resolution_clock::now();
            evaluator.rotate_rows_inplace(encrypted, 1, gal_keys);
            // evaluator.rotate_rows_inplace(encrypted, -1, gal_keys);
            time_end = chrono::high_resolution_clock::now();
            time_rotate_rows_one_step_sum += chrono::duration_cast<
                chrono::microseconds>(time_end - time_start);;

            /*
            [Rotate Rows Random]
            We rotate matrix rows by a random number of steps. This is much more
            expensive than rotating by just one step.
            */
            size_t row_size = batch_encoder.slot_count() / 2;
            int random_rotation = static_cast<int>(100000 % row_size);
            time_start = chrono::high_resolution_clock::now();
            evaluator.rotate_rows_inplace(encrypted, random_rotation, gal_keys);
            time_end = chrono::high_resolution_clock::now();
            time_rotate_rows_random_sum += chrono::duration_cast<
                chrono::microseconds>(time_end - time_start);

            /*
            [Rotate Columns]
            Nothing surprising here.
            */
            time_start = chrono::high_resolution_clock::now();
            evaluator.rotate_columns_inplace(encrypted, gal_keys);
            time_end = chrono::high_resolution_clock::now();
            time_rotate_columns_sum += chrono::duration_cast<
                chrono::microseconds>(time_end - time_start);
        }
        time_end_g = chrono::high_resolution_clock::now();
        time_diff_g = chrono::duration_cast<chrono::microseconds>(time_end_g - time_start_g);
        count = count + 1;
        if (time_diff_g.count() > MAXS) break;
        /*
        Print a dot to indicate progress.
        */
    }
    LogVVV << "count: " << count << endl;

    auto avg_batch = time_batch_sum.count();
    auto avg_unbatch = time_unbatch_sum.count();
    auto avg_encrypt = time_encrypt_sum.count();
    auto avg_decrypt = time_decrypt_sum.count();
    auto avg_add = time_add_sum.count();
    auto avg_multiply = time_multiply_sum.count();
    auto avg_multiply_plain = time_multiply_plain_sum.count();
    auto avg_square = time_square_sum.count();
    auto avg_relinearize = time_relinearize_sum.count();
    auto avg_rotate_rows_one_step = time_rotate_rows_one_step_sum.count();
    auto avg_rotate_rows_random = time_rotate_rows_random_sum.count();
    auto avg_rotate_columns = time_rotate_columns_sum.count();

    LogVVV << "Average batch: " << avg_batch << " microseconds" << endl;
    LogVVV << "Average unbatch: " << avg_unbatch << " microseconds" << endl;
    LogVVV << "Average encrypt: " << avg_encrypt << " microseconds" << endl;
    LogVVV << "Average decrypt: " << avg_decrypt << " microseconds" << endl;
    LogVVV << "Average add: " << avg_add << " microseconds" << endl;
    LogVVV << "Average multiply: " << avg_multiply << " microseconds" << endl;
    LogVVV << "Average multiply plain: " << avg_multiply_plain << " microseconds" << endl;
    LogVVV << "Average square: " << avg_square << " microseconds" << endl;
    if (context->using_keyswitching()){
        LogVVV << "Average relinearize: " << avg_relinearize << " microseconds" << endl;
        LogVVV << "Average rotate rows one step: " << avg_rotate_rows_one_step <<
            " microseconds" << endl;
        LogVVV << "Average rotate rows random: " << avg_rotate_rows_random <<
            " microseconds" << endl;
        LogVVV << "Average rotate columns: " << avg_rotate_columns <<
            " microseconds" << endl;
    }
    LogVVV.close();
    pthread_exit(NULL);
}

int muti_core_runner(){
    /*
    */
    bool invalid = true;
    int cpu_core_num = 40;
    size_t m_degree = 1024;
    vector<int> valid_degree = {1024, 2048, 4096, 8192, 16384, 32768};
    do{
        cout << "+---------------------------------------------------------+" << endl;
        cout << "| Multi Core Task                                         |" << endl;
        cout << "+---------------------------------------------------------+" << endl;
        cout << endl << ">Enter Number of Threads (1 ~ 40) or exit (0):";
        while (!(cin >> cpu_core_num) || (cpu_core_num <0 || cpu_core_num > 40));
        if (cpu_core_num == 0){invalid = false; continue;}
        cout << endl << ">Enter poly_modulus_degree 1024, 2048, 4096, 8192, 16384 or 32768:";
        while (!(cin >> m_degree));
        if (find(valid_degree.begin(), valid_degree.end(), m_degree) == valid_degree.end()){
            cout << "Invalid poly_modulus_degree" << endl;
            invalid = false;
            continue;
        }
        /*Initialized thread data 
        */
        pthread_t thread[cpu_core_num];
        struct thread_para th_para[cpu_core_num];
        EncryptionParameters parms(scheme_type::BFV);
        parms.set_poly_modulus_degree(m_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(m_degree));
        if (m_degree == 1024){
            parms.set_plain_modulus(12289);
        }else{
            parms.set_plain_modulus(786433);
        }
        for (int i = 0; i < cpu_core_num; i ++){
            // th_para[i].poly_degree = m_degree;
            th_para[i].fd = i;
            th_para[i].context = SEALContext::Create(parms);
        }
        /*Create Thread
        */
        for (int i = 0; i < cpu_core_num; i ++){
            pthread_create(&thread[i], NULL, bfv_performance, (void*)(&th_para[i]));
        }
        /*Join
        */
        for (int i = 0; i < cpu_core_num; i ++){
            pthread_join(thread[i], NULL);
        }
        system("python3 ../clustar/logAn.py");
        cout << endl << "Done, Check the report.txt in clustar/record" << endl << endl;
    }while (invalid);
    return 0;
}
