import os
import subprocess

def remove_file(filename) :
    if(os.path.exists(filename)):
       os.remove(filename)

def run_script(commands, filename="test.db"):
    remove_file("test.db")
    result = subprocess.run(
            ["./bin/harborDB", filename, "--test-mode"],
            input = "\n".join(commands)+"\n",
            capture_output= True,
            text = True
            )

    return result.stdout.strip().split("\n")

def assertList(output, expected_output) : 
    assert len(expected_output) == len(output), "Unequal lenght of output and expected output."
    assert output == expected_output

def test_insert_and_retrieves(): 
    output = run_script(["insert 1 user1 example.com","select", ".exit"])
    expected_output = ["Executed", "(1 user1 example.com)","Executed" ]  
    assertList(output, expected_output)

def test_error_on_table_full(): 
    input = [f'insert {i} user{i} user{i}.com' for i in range(1500)] + [".exit"]
    output = run_script(input)
    assert "Table Full" in output

def test_max_length_string_insert():
    long_username = "a"*32
    long_email = "b"*255
    output = run_script([f"insert 1 {long_username} {long_email}", "select" ,".exit"])
    expected_output =["Executed", f"(1 {long_username} {long_email})", "Executed"]
    assertList(output, expected_output)
     
def test_long_length_string_insert(): 
    long_username = "a" * 33
    long_email = "b" * 255
    result = run_script([f"insert 1 {long_username} {long_email}", ".exit"])
    assert "String too long" in result

def test_negative_id_insert(): 
    result = run_script(["insert -1 user1 example1.com", ".exit"])
    assert "Id must be positive" in result

def test_disk_persistance(): 
    remove_file("test_disk_persistance.db")
    result = run_script(["insert 1 user1 example1.com", ".exit"], "test_disk_persistance.db")
    assert "Executed" in result
    output = run_script(["select", ".exit"], "test_disk_persistance.db")
    expected_output = ["(1 user1 example1.com)", "Executed"]
    assertList(output, expected_output)
    remove_file("test_disk_persistance.db")

def test_print_btree_structure(): 
    input = [f'insert {i} user{i} example{i}.com' for i in range(3)] + ['.btree','.exit'];
    output = run_script(input)
    expected_output = [
        "Executed",
        "Executed",
        "Executed",
        "Tree:",
        "Leaf (size: 3)",
        " - 0: 0",
        " - 1: 1",
        " - 2: 2",
    ]
    assertList(output, expected_output);
