#include "common.h"


void calc_bfv_basic(){
    /*Prepare the encryptor
    */
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 2048;
    vector<int> valid_degree = {2048, 4096, 8192, 16384, 32768};
    // print_parameters(context);
    /*Extract number and operation from operation_str
    */
    bool invalid = true;
    int op;
    do{
        cout << "+---------------------------------------------------------+" << endl;
        cout << "| Naive Calculator V0.1                                   |" << endl;
        cout << "+---------------------------------------------------------+" << endl;
        cout << "| Type of operation          | Example                    |" << endl;
        cout << "+----------------------------+----------------------------+" << endl;
        cout << "| 1. Add Plain               | Operation: 1（Add）        |" << endl;
        cout << "| 2. Multiply                | Enter Number: 24           |" << endl;
        cout << "| 3. Square                  | Enter Number: 12           |" << endl;
        cout << "+----------------------------+----------------------------+" << endl;
        cout << endl << ">Enter poly_modulus_degree 2048, 4096, 8192, 16384 32768 or exit (0):";
        while (!(cin >> poly_modulus_degree));
        if (find(valid_degree.begin(), valid_degree.end(), poly_modulus_degree) == valid_degree.end()){
            cout << "Invalid poly_modulus_degree" << endl;
            invalid = false;
            continue;
        }
        if (poly_modulus_degree == 0){invalid = false; continue;}
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(1024);
        auto context = SEALContext::Create(parms);
        cout << endl << ">Enter Operation (1 ~ 3):";
        while (!(cin>>op) || ((op < 0 || op > 3)));
        switch(op){
            case 1: add_plain_helper(op, context); break;
            case 2: mul_helper(op, context); break;
            case 3: square_helper(op, context); break;
            default: break;
        }
    }while(invalid);//end for while
}

inline void add_plain_helper(int op, shared_ptr<SEALContext> context){
    int num1;
    int num2;
    cout << endl << ">Enter Number:"; 
    while(!(cin >> num1));
    cout << endl << ">Enter Number:";
    while (!(cin >> num2));
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff;
    size_t op_time, en_time, de_time, re_time = -1;
    /*Generate Key Pair
    */
    KeyGenerator keygen(context);
    PublicKey public_key = keygen.public_key();
    SecretKey secret_key = keygen.secret_key();

    /*Create encryptor and decryptor
    */
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    IntegerEncoder encoder(context);
    
    /*Encrypted the number
    */
    Plaintext x_plain_1 = encoder.encode(num1); 
    Plaintext x_plain_2 = encoder.encode(num2);
    Ciphertext x_encrypted_1, x_encrypted_2;
    time_start = chrono::high_resolution_clock::now();
    encryptor.encrypt(x_plain_1, x_encrypted_1);
    encryptor.encrypt(x_plain_2, x_encrypted_2);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    en_time = time_diff.count();

    time_start = chrono::high_resolution_clock::now();
    evaluator.add_inplace(x_encrypted_1, x_encrypted_2);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    op_time = time_diff.count();


    /*Size and noise Budget 
    */
    size_t size_encrypt = x_encrypted_1.size();
    size_t noise_budget = decryptor.invariant_noise_budget(x_encrypted_1);

    /*Decrypted the ciphertext
    */
    Plaintext res_plain;
    time_start = chrono::high_resolution_clock::now();
    decryptor.decrypt(x_encrypted_1, res_plain);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    de_time = time_diff.count();

    print_result(op, context, num1, num2, size_encrypt, noise_budget, res_plain.to_string(), encoder.decode_int32(res_plain), en_time, re_time, de_time, op_time);
}

inline void mul_helper(int op, shared_ptr<SEALContext> context){
    int num1;
    int num2;
    cout << endl << ">Enter Number:"; 
    while(!(cin >> num1));
    cout << endl << ">Enter Number:";
    while (!(cin >> num2));
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff;
    size_t op_time, en_time, de_time, re_time = -1;
    /*Generate Key Pair
    */
    KeyGenerator keygen(context);
    PublicKey public_key = keygen.public_key();
    SecretKey secret_key = keygen.secret_key();

    /*Create encryptor and decryptor
    */
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    IntegerEncoder encoder(context);
    
    /*Encrypted the number
    */
    Plaintext x_plain_1 = encoder.encode(num1); 
    Plaintext x_plain_2 = encoder.encode(num2);
    Ciphertext x_encrypted_1, x_encrypted_2;
    time_start = chrono::high_resolution_clock::now();
    encryptor.encrypt(x_plain_1, x_encrypted_1);
    encryptor.encrypt(x_plain_2, x_encrypted_2);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    en_time = time_diff.count();

    time_start = chrono::high_resolution_clock::now();
    evaluator.multiply_inplace(x_encrypted_1, x_encrypted_2);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    op_time = time_diff.count();
    

    /*Generalized Relinearize Key
    */
    if (context->using_keyswitching()){
        auto relin_keys = keygen.relin_keys();
        time_start = chrono::high_resolution_clock::now();
        evaluator.relinearize_inplace(x_encrypted_1, relin_keys);  
        time_end = chrono::high_resolution_clock::now();
        time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
        re_time = time_diff.count(); 
    }

    /*Size and noise Budget 
    */
    size_t size_encrypt = x_encrypted_1.size();
    size_t noise_budget = decryptor.invariant_noise_budget(x_encrypted_1);

    /*Decrypted the ciphertext
    */
    Plaintext res_plain;
    time_start = chrono::high_resolution_clock::now();
    decryptor.decrypt(x_encrypted_1, res_plain);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    de_time = time_diff.count();
    print_result(op, context, num1, num2, size_encrypt, noise_budget, res_plain.to_string(), encoder.decode_int32(res_plain), en_time, re_time, de_time, op_time);
}

inline void square_helper(int op, shared_ptr<SEALContext> context){
    int num1, num2 = -1;
    cout << endl << ">Enter Number:"; 
    while(!(cin >> num1));
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff;
    size_t op_time, en_time, de_time, re_time = -1;
    /*Generate Key Pair
    */
    KeyGenerator keygen(context);
    PublicKey public_key = keygen.public_key();
    SecretKey secret_key = keygen.secret_key();

    /*Create encryptor and decryptor
    */
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    IntegerEncoder encoder(context);
    
    /*Encrypted the number
    */
    Plaintext x_plain_1 = encoder.encode(num1); 
    Ciphertext x_encrypted_1;
    time_start = chrono::high_resolution_clock::now();
    encryptor.encrypt(x_plain_1, x_encrypted_1);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    en_time = time_diff.count();

    time_start = chrono::high_resolution_clock::now();
    evaluator.square_inplace(x_encrypted_1);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    op_time = time_diff.count();

    /*Generalized Relinearize Key
    */
    if (context->using_keyswitching()){
        auto relin_keys = keygen.relin_keys();
        time_start = chrono::high_resolution_clock::now();
        evaluator.relinearize_inplace(x_encrypted_1, relin_keys);  
        time_end = chrono::high_resolution_clock::now();
        time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
        re_time = time_diff.count();   
    }
    
    /*Size and noise Budget 
    */
    size_t size_encrypt = x_encrypted_1.size();
    size_t noise_budget = decryptor.invariant_noise_budget(x_encrypted_1);

    /*Decrypted the ciphertext
    */
    Plaintext res_plain;
    time_start = chrono::high_resolution_clock::now();
    decryptor.decrypt(x_encrypted_1, res_plain);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    de_time = time_diff.count();
    print_result(op, context, num1, num2, size_encrypt, noise_budget, res_plain.to_string(), encoder.decode_int32(res_plain), en_time, re_time, de_time, op_time);
}

