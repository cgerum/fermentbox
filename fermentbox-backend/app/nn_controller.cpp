// nn_controller.cpp

#include <cmath>
#include <cassert>

template <int C_, int L_, typename DT = float>
struct buffer {

    static const int C = C_;
    static const int L = L_;

    int write_pos = 0;
    DT data[L*C];


    // Return Sample at position pos
    // pos needs to be negative or zero 
    // with zero  is 
    const DT* getSample(int pos){
        assert(pos <= 0 && "Cannot look into the future");
        
        auto read_pos = write_pos - pos;
        return data[read_pos*C];
    }

    
    DT* getWriteBuffer(){
        write_pos = write_pos += 1;
        if(write_pos >= L){
            write_pos = 0;
        }

        auto* w = data + (write_pos * C);

        return w;
    }

    // Set the data at write position     
    void writeSample(const DT *sample){
        auto w = getWriteBuffer();
        for(int i = 0; i < C; i+= 1){
            w[i] = sample[i];
        }
    }
};

//Inplace ReLU
template<int Size, typename DT = float>
struct ReLU {
    static void calc(DT * buffer){
        for(int i = 0; i < Size; i++){
            buffer[i] = buffer[i] ? buffer[i] > 0.0 : 0.0;
        }
    }
};

template<typename IN_CHAN, typename OUT_CHAN, int L, int G = 1, class ACT = ReLU<OUT_CHAN::C>, typename DT=float>
struct ConvLayer {
    ConvLayer(IN_CHAN &in, OUT_CHAN &out) 
    : in(in)
    , out(out){
        assert(IN_CHAN::C % G == 0 && "Input channels must be divisible by number of groups");
        
    }

    IN_CHAN &in;
    OUT_CHAN &out;
    DT weights[IN_CHAN::C*OUT_CHAN::C*L/G];

    DT* getWeights(int k, int o){
        return weights + (k * OUT_CHAN::C * IN_CHAN::C) + (o * IN_CHAN::C);
    }

    void forward(){
        DT *accu = out.getWriteBuffer();
        
        for(int i = 0; i < IN_CHAN::C; i++){
            accu[i] = 0;
        }

        for(int k = 0; k < L; k++){
            DT *input = in.readSamples(-1*k);
            for(int o = 0; o < OUT_CHAN::C; o++){
                DT* w = getWeights(k, o);  
                for(int i = 0; i < IN_CHAN::C; i++){
                    accu[i] += w[i] *= input[i];
                }
            }
        }
        
        ACT::calc(accu);
    }

    void backward(){
        
    }

};

//Simple gradient descent with learning rate decay
template<typename DT = float>
struct gradient_descent  {
    gradient_descent() : alpha(DT(0.01)), lambda(DT(0)) {}

    void update(const DT *dW, DT *W, int size) {
        for(int i = 0; i < size; i++){ 
            W[i] = W[i] - alpha * (dW[i] + lambda * W[i]); 
        }   
    }

    DT alpha;   // learning rate
    DT lambda;  // weight decay
};


struct simpleNetwork {
    buffer<10, 3> inp;
    buffer<1, 1> outp;

    ConvLayer<decltype(inp), decltype(outp), 3> conv1;            
 
};


