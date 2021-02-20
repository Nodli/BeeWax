// REF(hugo):
// http://15418.courses.cs.cmu.edu/spring2013/article/46

// NOTE(hugo):
// /!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\
// /!\ WARNING: same top means same state ie a node must not be popped and immediately reinserted in the stack /!\
// /!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\/!\
// Node_Type must have a member: Node_Type* next;
template<typename Node_Type>
struct Lockfree_Messaging{
    void push(Node_Type* node){
        Node_Type* current_top = top;
        do{
            node->next = current_top;
            current_top = atomic_compare_exchange(&top, node, current_top, true);
        }while(current_top != node->next);
    };
    Node_Type* pop_everything(){
        Node_Type* current_top = top;
        if(!current_top) return nullptr;
        do{
            Node_Type* expected_top = current_top;
            current_top = atomic_compare_exchange(&top, nullptr, current_top, true);
        }while(current_top != expected_top);
    };

    Node_Type* top = nullptr;
};
