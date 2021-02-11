void artistic_pixel_sort(const char* input_path, const char* output_path,
        artistic_pixel_sort_parameters& param){

    int (*distance_function_ptr)(int, int, int, int);
    switch(param.distance_function){
        default:
        case MANHATTAN:
            distance_function_ptr = &distance_manhattan;
            break;
        case AXIAL_MULTIPLIED:
            distance_function_ptr = &distance_axial_multiplied;
            break;
        case SQUARED:
            distance_function_ptr = &distance_squared;
            break;
    }

    int width;
    int height;
    int channels;
    uchar* data = stbi_load(input_path, &width, &height, &channels, 3);
    assert(data);
    int image_size = width * height;

    char* inner_flag = (char*)calloc(image_size, sizeof(char));

    int originx = (int)((float)width * param.relative_originx);
    int originy = (int)((float)height * param.relative_originy);
    int iorigin = originx + originy * width;

    add_flag(inner_flag[iorigin], processed_flag);

    // NOTE(hugo): build the list of unprocessed ie the pool of available pixels
    int nouter = image_size - 1;
    int* outer = (int*)malloc(image_size * sizeof(int));
    for(int iprev = 0; iprev != iorigin; ++iprev){
        outer[iprev] = iprev;
    }
    for(int ipost = iorigin + 1; ipost != image_size; ++ipost){
        outer[ipost - 1] = ipost;
    }

    // NOTE(hugo): build the list of unprocessed adjacent ie the processing front
    int* front = (int*)malloc(image_size * sizeof(int));
    int nfront = neigh4_i2D(iorigin, width, image_size, front);

    int max_sample_size = param.collection_size;

    int progress_delta = image_size / 100;

    while(nouter > 0){
        if(nouter % progress_delta == 0){
            printf("%d percent \n", 100 - nouter / progress_delta);
        }

        // 1. Select a random collection of unprocessed pixels.
        //    * Each pixel in the collection must be adjacent to an already processed pixel.
        int front_permutation_size = std::min(nfront, max_sample_size);
        random_partial_permutation_in_place(front, nfront, front_permutation_size);

        // 2. Pick the pixel with the longest path back to the point of origin.
        //    * Let's call it P.
        int P = 0;
        int P_dist = INT_MAX;
        for(int icandidate = 0; icandidate != front_permutation_size; ++icandidate){
            int candidate_x, candidate_y;
            c2D(front[icandidate], width, candidate_x, candidate_y);
            int candidate_dist = distance_function_ptr(candidate_x, candidate_y, originx, originy);
            if(candidate_dist < P_dist){
                P_dist = candidate_dist;
                P = front[icandidate];
            }
        }

        // 3. Select a new collection of pixels among the unprocessed ones.
        //    * These don't have the adjacency requirement.
        int outer_permutation_size = std::min(nouter, max_sample_size);
        random_partial_permutation_in_place(outer, nouter, outer_permutation_size);

        // 4. Pick the pixel most similar to P's adjacent processed pixels.
        //    * Let's call it R
        int P_neigh[4];
        int P_neigh_count = neigh4_i2D(P, width, image_size, P_neigh);
        int P_neigh_inner[4];
        int P_neigh_inner_count = 0;
        int P_neigh_other[4];
        int P_neigh_other_count = 0;
        for(int ineigh = 0; ineigh != P_neigh_count; ++ineigh){
            int neigh = P_neigh[ineigh];
            if(check_flag(inner_flag[neigh], processed_flag)){
                P_neigh_inner[P_neigh_inner_count++] = neigh;
            }else{
                P_neigh_other[P_neigh_other_count++] = neigh;
            }
        }

        int R = 0;
        int R_cost = INT_MAX;
        for(int icandidate = 0; icandidate != outer_permutation_size; ++icandidate){
            int candidate_cost = 0;
            uchar* candidate_color = data + 3 * outer[icandidate];

            for(int ineigh_inner = 0; ineigh_inner != P_neigh_inner_count; ++ineigh_inner){
                uchar* neigh_color = data + 3 * P_neigh_inner[ineigh_inner];
                candidate_cost += color_distance(candidate_color, neigh_color);
            }

            if(candidate_cost < R_cost){
                R_cost = candidate_cost;
                R = outer[icandidate];
            }
        }

        // 5. Swap the colours of P and R.
        //    * P is now processed
        uchar temp_color[3] = {data[3 * P], data[3 * P + 1], data[3 * P + 2]};
        data[3 * P] = data[3 * R];
        data[3 * P + 1] = data[3 * R + 1 ];
        data[3 * P + 2] = data[3 * R + 2];
        data[3 * R] = temp_color[0];
        data[3 * R + 1] = temp_color[1];
        data[3 * R + 2] = temp_color[2];

        // remove P from the unprocessed and adjacent
        add_flag(inner_flag[P], processed_flag);
        for(int iouter = 0; iouter != nouter; ++iouter){
            if(outer[iouter] == P){
                memmove(outer + iouter, outer + iouter + 1, (nouter - iouter - 1) * sizeof(int));
                --nouter;
                break;
            }
        }
        for(int ifront = 0; ifront != nfront; ++ifront){
            if(front[ifront] == P){
                memmove(front + ifront, front + ifront + 1, (nfront - ifront - 1) * sizeof(int));
                --nfront;
                break;
            }
        }

        // add P's adjacency to the front
        bool P_neigh_other_in_front[4] = {};
        for(int ifront = 0; ifront != nfront; ++ifront){
            for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
                if(front[ifront] == P_neigh_other[ineigh_other]){
                    P_neigh_other_in_front[ineigh_other] = true;
                }
            }
        }
        for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
            if(!P_neigh_other_in_front[ineigh_other]){
                front[nfront++] = P_neigh_other[ineigh_other];
            }
        }
    }

    stbi_write_png(output_path, width, height, 3, data, 0);

    free(data);
    free(inner_flag);
    free(outer);
    free(front);
}

struct artistic_pixel_sort_propagation_parameters{
    float relative_originx = 0.f;
    float relative_originy = 0.f;
    int collection_size = 10;
};

void artistic_pixel_sort_propagation(const char* input_path, const char* output_path,
        artistic_pixel_sort_propagation_parameters& param){

    int width;
    int height;
    int channels;
    uchar* data = stbi_load(input_path, &width, &height, &channels, 3);
    assert(data);
    int image_size = width * height;

    // is_processed(i) = (inner_distance[i] < 0)
    int* inner_distance = (int*)calloc(image_size, sizeof(int));

    int originx = 0;//(int)((float)width * param.relative_originx);
    int originy = 0;//(int)((float)height * param.relative_originy);
    int iorigin = originx + originy * width;

    inner_distance[iorigin] = - 1;

    // NOTE(hugo): build the list of unprocessed ie the pool of available pixels
    int nouter = image_size - 1;
    int* outer = (int*)malloc(image_size * sizeof(int));
    for(int iprev = 0; iprev != iorigin; ++iprev){
        outer[iprev] = iprev;
    }
    for(int ipost = iorigin + 1; ipost != image_size; ++ipost){
        outer[ipost - 1] = ipost;
    }

    // NOTE(hugo): build the list of unprocessed adjacent ie the processing front
    int* front = (int*)malloc(image_size * sizeof(int));
    int nfront = neigh4_i2D(iorigin, width, image_size, front);
    for(int ifront = 0; ifront != nfront; ++ifront){
        inner_distance[front[ifront]] = - inner_distance[iorigin] + 1;
    }

    int max_sample_size = param.collection_size;

    int progress_delta = image_size / 100;

    while(nouter > 0){
        if(progress_delta){
            if(nouter % progress_delta == 0){
                printf("%d percent\n", 100 - nouter / progress_delta);
                if(nouter / progress_delta % 5 == 0){
                    stbi_write_png(output_path, width, height, 3, data, 0);
                    SDL_Delay(1000);
                }
            }
        }else{
            printf("%d pixels remaining\n", nouter);
        }

        // 1. Select a random collection of unprocessed pixels.
        //    * Each pixel in the collection must be adjacent to an already processed pixel.
        int front_permutation_size = std::min(nfront, 5);
        random_partial_permutation_in_place(front, nfront, front_permutation_size);

        // 2. Pick the pixel with the longest path back to the point of origin.
        //    * Let's call it P.
        int P = 0;
        int P_dist = 0;
        for(int icandidate = 0; icandidate != front_permutation_size; ++icandidate){
            int candidate = front[icandidate];
            int candidate_dist = inner_distance[candidate];
            if(candidate_dist > P_dist){
                P = candidate;
                P_dist = candidate_dist;
            }
        }

        // 3. Select a new collection of pixels among the unprocessed ones.
        //    * These don't have the adjacency requirement.
        int outer_permutation_size = std::min(nouter, 1000);
        random_partial_permutation_in_place(outer, nouter, outer_permutation_size);

        // 4. Pick the pixel most similar to P's adjacent processed pixels.
        //    * Let's call it R
        int P_neigh[4];
        int P_neigh_count = neigh4_i2D(P, width, image_size, P_neigh);
        int P_neigh_inner[4];
        int P_neigh_inner_count = 0;
        int P_neigh_other[4];
        int P_neigh_other_count = 0;
        for(int ineigh = 0; ineigh != P_neigh_count; ++ineigh){
            int neigh = P_neigh[ineigh];
            if(inner_distance[neigh] < 0){
                P_neigh_inner[P_neigh_inner_count++] = neigh;
            }else{
                P_neigh_other[P_neigh_other_count++] = neigh;
            }
        }
        assert(P_neigh_inner_count > 0);

        int R = 0;
        int R_cost = INT_MAX;
        for(int icandidate = 0; icandidate != outer_permutation_size; ++icandidate){
            int candidate_cost = 0;
            uchar* candidate_color = data + 3 * outer[icandidate];

            for(int ineigh_inner = 0; ineigh_inner != P_neigh_inner_count; ++ineigh_inner){
                uchar* neigh_color = data + 3 * P_neigh_inner[ineigh_inner];
                candidate_cost += color_distance(candidate_color, neigh_color);
            }

            if(candidate_cost < R_cost){
                R_cost = candidate_cost;
                R = outer[icandidate];
            }
        }

        // 5. Swap the colours of P and R.
        //    * P is now processed
        uchar temp_color[3] = {data[3 * P], data[3 * P + 1], data[3 * P + 2]};
        data[3 * P] = data[3 * R];
        data[3 * P + 1] = data[3 * R + 1];
        data[3 * P + 2] = data[3 * R + 2];
        data[3 * R] = temp_color[0];
        data[3 * R + 1] = temp_color[1];
        data[3 * R + 2] = temp_color[2];

        // remove P from the unprocessed and adjacent
        inner_distance[P] = - inner_distance[P];
        for(int iouter = 0; iouter != nouter; ++iouter){
            if(outer[iouter] == P){
                memmove(outer + iouter, outer + iouter + 1, (nouter - iouter - 1) * sizeof(int));
                --nouter;
                break;
            }
        }
        for(int ifront = 0; ifront != nfront; ++ifront){
            if(front[ifront] == P){
                memmove(front + ifront, front + ifront + 1, (nfront - ifront - 1) * sizeof(int));
                --nfront;
                break;
            }
        }

        // add P's adjacency to the front
        bool P_neigh_other_in_front[4] = {};
        for(int ifront = 0; ifront != nfront; ++ifront){
            for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
                if(front[ifront] == P_neigh_other[ineigh_other]){
                    P_neigh_other_in_front[ineigh_other] = true;
                }
            }
        }
        for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
            if(!P_neigh_other_in_front[ineigh_other]){
                front[nfront++] = P_neigh_other[ineigh_other];
                inner_distance[P_neigh_other[ineigh_other]] = - inner_distance[P] + 1;
            }
        }
    }

    stbi_write_png(output_path, width, height, 3, data, 0);

    free(data);
    free(inner_distance);
    free(outer);
    free(front);
}

void artistic_pixel_sort_propagation(const char* input_path, const char* output_path,
        artistic_pixel_sort_propagation_parameters& param){

    int width;
    int height;
    int channels;
    uchar* data = stbi_load(input_path, &width, &height, &channels, 3);
    assert(data);
    int image_size = width * height;

    // is_processed(i) = (inner_distance[i] < 0)
    int* inner_distance = (int*)calloc(image_size, sizeof(int));

    int originx = width / 2;
    int originy = height / 2;
    int iorigin = originx + originy * width;

    inner_distance[iorigin] = - 1;

    // NOTE(hugo): build the list of unprocessed ie the pool of available pixels
    //             and the cumulated weights (based on the color ie updated for each P)
    int nouter = image_size - 1;
    int* outer = (int*)malloc(image_size * sizeof(int));
    int* outer_cweight = (int*)malloc(image_size * sizeof(int));
    for(int iprev = 0; iprev != iorigin; ++iprev){
        outer[iprev] = iprev;
    }
    for(int ipost = iorigin + 1; ipost != image_size; ++ipost){
        outer[ipost - 1] = ipost;
    }

    // NOTE(hugo): build the list of unprocessed adjacent ie the processing front
    //             and the cumulated weights (based on the distance ie updated when adding / removing)
    int* front = (int*)malloc(image_size * sizeof(int));
    int* front_cweight = (int*)malloc(image_size * sizeof(int));
    int nfront = neigh4_i2D(iorigin, width, image_size, front);
    for(int ifront = 0; ifront != nfront; ++ifront){
        inner_distance[front[ifront]] = - inner_distance[iorigin] + 1;
    }
    front_cweight[0] = inner_distance[front[0]];
    for(int ifront = 1; ifront < nfront; ++ifront){
        front_cweight[ifront] = front_cweight[ifront - 1] + inner_distance[front[ifront]];
    }

    int progress_delta = image_size / 100;

    while(nouter > 0){
        if(progress_delta){
            if(nouter % progress_delta == 0){
                printf("%d percent\n", 100 - nouter / progress_delta);
                if(nouter / progress_delta % 5 == 0){
                    stbi_write_png(output_path, width, height, 3, data, 0);
                    SDL_Delay(1000);
                }
            }
        }else{
            printf("%d pixels remaining\n", nouter);
        }

        int P = 0;
        int P_index = 0;

        // Pick the pixel of the front
        // The fursthest the pixel is from the origin, the more likely it is to be picked
        int random_cweight = random_uint_range_uniform(front_cweight[nfront - 1]);
        for(int ifront = 0; ifront != nfront; ++ifront){
            if(random_cweight < front_cweight[ifront]){
                P = front[ifront];
                P_index = ifront;
                break;
            }
        }

        // Find P's neighbors
        int P_neigh[4];
        int P_neigh_count = neigh4_i2D(P, width, image_size, P_neigh);
        int P_neigh_inner[4];
        int P_neigh_inner_count = 0;
        int P_neigh_other[4];
        int P_neigh_other_count = 0;
        for(int ineigh = 0; ineigh != P_neigh_count; ++ineigh){
            int neigh = P_neigh[ineigh];
            if(inner_distance[neigh] < 0){
                P_neigh_inner[P_neigh_inner_count++] = neigh;
            }else{
                P_neigh_other[P_neigh_other_count++] = neigh;
            }
        }
        assert(P_neigh_inner_count > 0);

        // Update the outer pixels weights using the color difference
        for(int iouter = 0; iouter != nouter; ++iouter){
            uchar* outer_color = data + 3 * outer[iouter];

            int cost = INT_MAX;
            for(int ineigh_inner = 0; ineigh_inner != P_neigh_inner_count; ++ineigh_inner){
                uchar* neigh_color = data + 3 * P_neigh_inner[ineigh_inner];
                int neigh_cost = color_distance(outer_color, neigh_color);
                cost += std::min(cost, neigh_cost);
            }

#if 0
            int cost = 0;
            for(int ineigh_inner = 0; ineigh_inner != P_neigh_inner_count; ++ineigh_inner){
                uchar* neigh_color = data + 3 * P_neigh_inner[ineigh_inner];
                int neigh_cost = color_distance(outer_color, neigh_color);
                cost += neigh_cost;
            }
#endif
            outer_cweight[iouter] = cost;
        }

        int R = 0;

        // Pick the pixel of the outer
        int R_cost = INT_MAX;
        for(int iouter = 0; iouter != nouter; ++iouter){
            if(outer_cweight[iouter] < R_cost){
                R = outer[iouter];
                R_cost = outer_cweight[iouter];
            }
        }

        // 5. Swap the colours of P and R.
        //    * P is now processed
        uchar temp_color[3] = {data[3 * P], data[3 * P + 1], data[3 * P + 2]};
        data[3 * P] = data[3 * R];
        data[3 * P + 1] = data[3 * R + 1];
        data[3 * P + 2] = data[3 * R + 2];
        data[3 * R] = temp_color[0];
        data[3 * R + 1] = temp_color[1];
        data[3 * R + 2] = temp_color[2];

        // remove P from the unprocessed and adjacent
        inner_distance[P] = - inner_distance[P];
        for(int iouter = 0; iouter != nouter; ++iouter){
            if(outer[iouter] == P){
                memmove(outer + iouter, outer + iouter + 1, (nouter - iouter - 1) * sizeof(int));
                --nouter;
                break;
            }
        }
        memmove(front + P_index, front + P_index + 1, (nfront - P_index - 1) * sizeof(int));
        int P_weight = - inner_distance[P];
        for(int ifront = P_index; ifront != nfront - 1; ++ifront){
            front_cweight[ifront] = front_cweight[ifront + 1] - P_weight;
        }
        --nfront;

        // add P's adjacency to the front
        bool P_neigh_other_in_front[4] = {};
        for(int ifront = 0; ifront != nfront; ++ifront){
            for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
                if(front[ifront] == P_neigh_other[ineigh_other]){
                    P_neigh_other_in_front[ineigh_other] = true;
                }
            }
        }
        for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
            if(!P_neigh_other_in_front[ineigh_other]){
                inner_distance[P_neigh_other[ineigh_other]] = - inner_distance[P] + 1;

                front[nfront] = P_neigh_other[ineigh_other];
                front_cweight[nfront] = front_cweight[nfront - 1] + inner_distance[P_neigh_other[ineigh_other]];
                ++nfront;
            }
        }
    }

    stbi_write_png(output_path, width, height, 3, data, 0);

    free(data);
    free(inner_distance);
    free(outer);
    free(front);
}

void artistic_pixel_sort_propagation(const char* input_path, const char* output_path,
        artistic_pixel_sort_propagation_parameters& param){

    int width;
    int height;
    int channels;
    uchar* data = stbi_load(input_path, &width, &height, &channels, 3);
    assert(data);
    int image_size = width * height;

    // is_processed(i) = (inner_distance[i] < 0)
    int* inner_distance = (int*)calloc(image_size, sizeof(int));

    int originx = width / 2;
    int originy = height / 2;
    int iorigin = originx + originy * width;

    inner_distance[iorigin] = - 1;

    // NOTE(hugo): build the list of unprocessed ie the pool of available pixels
    //             and the cumulated weights (based on the color ie updated for each P)
    int nouter = image_size - 1;
    int* outer = (int*)malloc(image_size * sizeof(int));
    int* outer_cweight = (int*)malloc(image_size * sizeof(int));
    for(int iprev = 0; iprev != iorigin; ++iprev){
        outer[iprev] = iprev;
    }
    for(int ipost = iorigin + 1; ipost != image_size; ++ipost){
        outer[ipost - 1] = ipost;
    }

    // NOTE(hugo): build the list of unprocessed adjacent ie the processing front
    //             and the cumulated weights (based on the distance ie updated when adding / removing)
    int* front = (int*)malloc(image_size * sizeof(int));
    int* front_cweight = (int*)malloc(image_size * sizeof(int));
    int nfront = neigh4_i2D(iorigin, width, image_size, front);
    for(int ifront = 0; ifront != nfront; ++ifront){
        inner_distance[front[ifront]] = - inner_distance[iorigin] + 1;
    }
    front_cweight[0] = inner_distance[front[0]];
    for(int ifront = 1; ifront < nfront; ++ifront){
        front_cweight[ifront] = front_cweight[ifront - 1] + inner_distance[front[ifront]];
    }

    int progress_delta = image_size / 100;

    while(nouter > 0){
        if(progress_delta){
            if(nouter % progress_delta == 0){
                printf("%d percent\n", 100 - nouter / progress_delta);
                if(nouter / progress_delta % 5 == 0){
                    stbi_write_png(output_path, width, height, 3, data, 0);
                    SDL_Delay(1000);
                }
            }
        }else{
            printf("%d pixels remaining\n", nouter);
        }

        int P = 0;
        int P_index = 0;

        // Pick the pixel of the front
        // The fursthest the pixel is from the origin, the more likely it is to be picked
        int random_cweight = random_uint_range_uniform(front_cweight[nfront - 1]);
        for(int ifront = 0; ifront != nfront; ++ifront){
            if(random_cweight < front_cweight[ifront]){
                P = front[ifront];
                P_index = ifront;
                break;
            }
        }

        // Find P's neighbors
        int P_neigh[4];
        int P_neigh_count = neigh4_i2D(P, width, image_size, P_neigh);
        int P_neigh_inner[4];
        int P_neigh_inner_count = 0;
        int P_neigh_other[4];
        int P_neigh_other_count = 0;
        for(int ineigh = 0; ineigh != P_neigh_count; ++ineigh){
            int neigh = P_neigh[ineigh];
            if(inner_distance[neigh] < 0){
                P_neigh_inner[P_neigh_inner_count++] = neigh;
            }else{
                P_neigh_other[P_neigh_other_count++] = neigh;
            }
        }
        assert(P_neigh_inner_count > 0);

        // Update the outer pixels weights using the color difference
        for(int iouter = 0; iouter != nouter; ++iouter){
            uchar* outer_color = data + 3 * outer[iouter];

            int cost = INT_MAX;
            for(int ineigh_inner = 0; ineigh_inner != P_neigh_inner_count; ++ineigh_inner){
                uchar* neigh_color = data + 3 * P_neigh_inner[ineigh_inner];
                int neigh_cost = color_distance(outer_color, neigh_color);
                cost += std::min(cost, neigh_cost);
            }

#if 0
            int cost = 0;
            for(int ineigh_inner = 0; ineigh_inner != P_neigh_inner_count; ++ineigh_inner){
                uchar* neigh_color = data + 3 * P_neigh_inner[ineigh_inner];
                int neigh_cost = color_distance(outer_color, neigh_color);
                cost += neigh_cost;
            }
#endif
            outer_cweight[iouter] = cost;
        }

        int R = 0;

        // Pick the pixel of the outer
        int R_cost = INT_MAX;
        for(int iouter = 0; iouter != nouter; ++iouter){
            if(outer_cweight[iouter] < R_cost){
                R = outer[iouter];
                R_cost = outer_cweight[iouter];
            }
        }

        // 5. Swap the colours of P and R.
        //    * P is now processed
        uchar temp_color[3] = {data[3 * P], data[3 * P + 1], data[3 * P + 2]};
        data[3 * P] = data[3 * R];
        data[3 * P + 1] = data[3 * R + 1];
        data[3 * P + 2] = data[3 * R + 2];
        data[3 * R] = temp_color[0];
        data[3 * R + 1] = temp_color[1];
        data[3 * R + 2] = temp_color[2];

        // remove P from the unprocessed and adjacent
        inner_distance[P] = - inner_distance[P];
        for(int iouter = 0; iouter != nouter; ++iouter){
            if(outer[iouter] == P){
                memmove(outer + iouter, outer + iouter + 1, (nouter - iouter - 1) * sizeof(int));
                --nouter;
                break;
            }
        }
        memmove(front + P_index, front + P_index + 1, (nfront - P_index - 1) * sizeof(int));
        int P_weight = - inner_distance[P];
        for(int ifront = P_index; ifront != nfront - 1; ++ifront){
            front_cweight[ifront] = front_cweight[ifront + 1] - P_weight;
        }
        --nfront;

        // add P's adjacency to the front
        bool P_neigh_other_in_front[4] = {};
        for(int ifront = 0; ifront != nfront; ++ifront){
            for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
                if(front[ifront] == P_neigh_other[ineigh_other]){
                    P_neigh_other_in_front[ineigh_other] = true;
                }
            }
        }
        for(int ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
            if(!P_neigh_other_in_front[ineigh_other]){
                inner_distance[P_neigh_other[ineigh_other]] = - inner_distance[P] + 1;

                front[nfront] = P_neigh_other[ineigh_other];
                front_cweight[nfront] = front_cweight[nfront - 1] + inner_distance[P_neigh_other[ineigh_other]];
                ++nfront;
            }
        }
    }

    stbi_write_png(output_path, width, height, 3, data, 0);

    free(data);
    free(inner_distance);
    free(outer);
    free(front);
}

void something(const char* input_path, const char* output_path,
        u32 originX, u32 originY,
        float pixel_selection_sampling_ratio,
        float color_selection_sampling_ratio,
        float progress_message_frequency = 0.05f
        ){


    // NOTE(hugo): image loading
    int stbi_width;
    int stbi_height;
    int stbi_channels;
    uchar* data = stbi_load(input_path, &stbi_width, &stbi_height, &stbi_channels, 3);
    assert(stbi_width > 0 && stbi_height > 0);
    assert(data);
    DEFER{
        free(data);
    };

    u32 width = (u32)stbi_width;
    u32 height = (u32)stbi_height;
    u32 image_size = width * height;

    assert(originX < width && originY < height);
    u32 iorigin = originX + originY * width;

    // NOTE(hugo): converting parameters
    assert(pixel_selection_sampling_ratio > 0.f && pixel_selection_sampling_ratio <= 1.f);
    u32 pixel_selection_sampling_size = cst::clamp((u32)(pixel_selection_sampling_ratio * (float)image_size), 1u, image_size);

    assert(color_selection_sampling_ratio > 0.f && color_selection_sampling_ratio <= 1.f);
    u32 color_selection_sampling_size = cst::clamp((u32)(color_selection_sampling_ratio * (float)image_size), 1u, image_size);

    assert(progress_message_frequency > 0.f && progress_message_frequency < 1.f);
    u32 progress_message_frequency_iteration = (u32)(progress_message_frequency * (float)image_size);

    LOG_TRACE("pixel, color %d %d", pixel_selection_sampling_size, color_selection_sampling_size);

    // NOTE(hugo): inner / processed
    s32* inner_distance = (s32*)calloc(image_size, sizeof(s32));
    assert(inner_distance);
    DEFER{
        free(inner_distance);
    };

    inner_distance[iorigin] = -1;
    auto is_processed = [](u32 index, s32* inner_distance_array){
        return inner_distance_array[index] < 0;
    };

    // NOTE(hugo): outer / unprocessed
    u32 nouter = (u32)image_size - 1;
    u32* outer = (u32*)malloc(image_size * sizeof(u32));
    assert(outer);
    DEFER{
        free(outer);
    };

    for(u32 iprev = 0; iprev != iorigin; ++iprev){
        outer[iprev] = iprev;
    }
    for(u32 ipost = iorigin + 1; ipost != image_size; ++ipost){
        outer[ipost - 1] = ipost;
    }

    // NOTE(hugo): processing front / outer that are adjacent to inner
    u32* front = (u32*)malloc(image_size * sizeof(u32));
    assert(front);
    DEFER{
        free(front);
    };

    u32 nfront = neigh4_i2D(iorigin, width, image_size, front);
    for(u32 ifront = 0; ifront != nfront; ++ifront){
        inner_distance[front[ifront]] = - inner_distance[iorigin] + 1;
    }

    // NOTE(hugo): front neighbors
    u32* front_neighbors = (u32*)malloc(image_size * sizeof(u32));
    assert(front_neighbors);
    DEFER{
        free(front_neighbors);
    };

    for(u32 ifront = 0; ifront != nfront; ++ifront){
        front_neighbors[front[ifront]] = 1;
    }

    while(nouter > 0){
        // NOTE(hugo): emit progress message
        if(progress_message_frequency_iteration > 0 && nouter % progress_message_frequency_iteration == 0){
            LOG_TRACE("%f percent processed", 100.f - 100.f * (float)nouter / (float)progress_message_frequency_iteration * progress_message_frequency);
            stbi_write_png(output_path, (int)width, (int)height, 3, data, 0);
        }

        u32 P = 0;
        u32 P_front_index = 0;

        // 1'. Pick an unprocessed pixel with 3 or more neighbors if there is one in the front
        for(u32 ifront = 0; ifront != nfront; ++ifront){
            if(ifront
        }


        // 1. Select a random collection of unprocessed pixels.
        //    * Each pixel in the collection must be adjacent to an already processed pixel.
        u32 front_permutation_size = std::min(nfront, pixel_selection_sampling_size);
        random_partial_permutation_in_place(front, nfront, front_permutation_size);

        // 2. Pick the pixel with the longest path back to the point of origin.
        //    * Let's call it P.
        u32 P_dist = 0;
        for(u32 icandidate = 0; icandidate != front_permutation_size; ++icandidate){
            u32 candidate_x, candidate_y;
            c2D(front[icandidate], width, candidate_x, candidate_y);

            assert(inner_distance[front[icandidate]] > 0);
            u32 candidate_dist = (u32)inner_distance[front[icandidate]];

            if(candidate_dist > P_dist){
                P = front[icandidate];
                P_front_index = icandidate;
                P_dist = candidate_dist;
            }
        }

        // 3. Select a new collection of pixels among the unprocessed ones.
        //    * These don't have the adjacency requirement.
        u32 outer_permutation_size = std::min(nouter, color_selection_sampling_size);
        random_partial_permutation_in_place(outer, nouter, outer_permutation_size);

        // NOTE(hugo): classifying P's neighbors to speed up the selection of R
        u32 P_neigh[4];
        u32 P_neigh_count = neigh4_i2D(P, width, image_size, P_neigh);

        u32 P_neigh_inner[4];
        u32 P_neigh_inner_count = 0;
        u32 P_neigh_other[4];
        u32 P_neigh_other_count = 0;
        for(u32 ineigh = 0; ineigh != P_neigh_count; ++ineigh){
            u32 neigh = P_neigh[ineigh];
            if(is_processed(neigh, inner_distance)){
                P_neigh_inner[P_neigh_inner_count++] = neigh;
            }else{
                P_neigh_other[P_neigh_other_count++] = neigh;
            }
        }

        auto color_distance_function = [](uchar* A, uchar* B){
            u32 dR = (u32)A[0] - (u32)B[0];
            u32 dG = (u32)A[1] - (u32)B[1];
            u32 dB = (u32)A[2] - (u32)B[2];
            return dR * dR + dG * dG + dB * dB;
        };

        // 4. Pick the pixel most similar to P's adjacent processed pixels.
        //    * Let's call it R
        u32 R = 0;
        u32 R_outer_index = 0;
        u32 R_cost = INT_MAX;
        for(u32 icandidate = 0; icandidate != outer_permutation_size; ++icandidate){
            u32 candidate_cost = 0;
            uchar* candidate_color = data + 3 * outer[icandidate];

            // TODO(hugo): try using std::min here or the median ? to allow for differences while keeping similarity
            for(u32 ineigh_inner = 0; ineigh_inner != P_neigh_inner_count; ++ineigh_inner){
                uchar* neigh_color = data + 3 * P_neigh_inner[ineigh_inner];
                candidate_cost += color_distance_function(candidate_color, neigh_color);
            }

            if(candidate_cost < R_cost){
                R = outer[icandidate];
                R_outer_index = icandidate;
                R_cost = candidate_cost;
            }
        }

        UNUSED(R_outer_index);

        // 5. Swap the colours of P and R.
        //    * P is now processed
        uchar temp_color[3] = {data[3 * P], data[3 * P + 1], data[3 * P + 2]};
        data[3 * P] = data[3 * R];
        data[3 * P + 1] = data[3 * R + 1 ];
        data[3 * P + 2] = data[3 * R + 2];
        data[3 * R] = temp_color[0];
        data[3 * R + 1] = temp_color[1];
        data[3 * R + 2] = temp_color[2];

        // NOTE(hugo):  * add P to the inner
        //              * remove P from the front
        //              * remove P from the outer
        inner_distance[P] = - inner_distance[P];

        front[P_front_index] = front[nfront - 1];
        --nfront;

        for(u32 iouter = 0; iouter != nouter; ++iouter){
            if(outer[iouter] == P){
                outer[iouter] = outer[nouter - 1];
                break;
            }
        }
        -- nouter;

        // NOTE(hugo): include P's adjacency in the front
        bool P_neigh_other_in_front[4] = {false, false, false, false};
        for(u32 ifront = 0; ifront != nfront; ++ifront){
            for(u32 ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
                if(front[ifront] == P_neigh_other[ineigh_other]){
                    P_neigh_other_in_front[ineigh_other] = true;
                }
            }
        }
        for(u32 ineigh_other = 0; ineigh_other != P_neigh_other_count; ++ineigh_other){
            if(!P_neigh_other_in_front[ineigh_other]){
                inner_distance[P_neigh_other[ineigh_other]] = - inner_distance[P] + 1;
                front[nfront++] = P_neigh_other[ineigh_other];
            }
        }
    }

    LOG_TRACE("DONE");
    stbi_write_png(output_path, (int)width, (int)height, 3, data, 0);
}

int main(){
    something("./data/joconde.png", "./output/something.png", 0, 0, 0.00005f, 0.00005f);
    return 0;
}
