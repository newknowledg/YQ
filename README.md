# YQ

yq is a yaml parser modeled after the jq parser.

yq has limited yaml compatiblity. yq is designed to run only one yaml config at a time
for this reason the starting and ending of the yaml body ... and --- are not supported.

yq has been tested with AWS Cloudformation and Ansible config files.

yq currently does not support the Fn:: varaible replacement logic of Cloudformation.yg

## Installation

run gcc main.c -o yq

## How to use

./yq <query> <file/path>

### flags

-i <int> Set indentation

-c       Highlights keys

## examples

YAML Body

    key : value
    nested : 
        key : value
    array: 
      - first_element
      - |
        multi-line array 
        element
    multii-line_key : |
        multi-line
        string value
    nested_array :
        - array_key: value
          nested_array:
            key : value
---

    ./yq . example.yaml
prints out the the entire yaml body

    ./yq .nested.key example.yaml

prints the key and value of the element selected

    ./yq ".key | = 'new value'"

replaces the value of the selected key and prints out the entire yaml body with the new value

    ./yq "(.key | = 'new value') (.array[0] | ='new element')"  example.yaml

brackets allow to perform multiple actions in the same query

## ToDo

1. Inserting key value pairs into existing yaml structures
2. Extracting values from keys and assigning the values to new keys in a new yaml body
3. Logic for adding nested key values
