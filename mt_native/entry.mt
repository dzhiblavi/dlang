% intermediate lang; should be preprocessed before mt

start: __native_entry
accept: __native_fin
reject: __native_err
blank: _

% main entry; program starts with `main' function with zero arguments

__native_entry * -> __native_entry0 * <
__native_entry _ -> func_main $ ^
__native_entry0 _ -> func_main $ ^
