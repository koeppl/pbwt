use std::fs::File;
use std::io::{self, BufRead};
use std::collections::HashMap;
use byteorder::{LittleEndian, WriteBytesExt};
use std::io::BufReader;
#[macro_use] extern crate more_asserts;

#[macro_use] extern crate clap;


fn main() {
     let matches = clap_app!(myapp =>
         (@arg input: -i "Sets the input file to use")
         (@arg output: -o "Sets the output file to use")
     ).get_matches();
    
     let reader : Box<dyn BufRead> =  match matches.value_of("input") {
         None => Box::new(BufReader::new(io::stdin())),
         Some(filename) => Box::new(BufReader::new(File::open(filename).unwrap()))
       };
     let mut writer : Box<dyn std::io::Write> = match matches.value_of("input") {
         None => Box::new(io::stdout()),
         Some(filename) => Box::new(File::open(filename).unwrap())
       };
     // let reader = io::stdin();
     // let mut writer = io::stdout();

    struct HashGrammar {
        name : u64,
        size : u64,
        left : u64,
        right: u64,
    }
    let mut hash2index = HashMap::new(); //@ maps a non-terminal hash value to an index of `variables`
    let mut variables = Vec::new(); //@ maps index to non-terminal hash values

    let lines = io::BufReader::new(reader).lines();
    for line_io in lines {
        if let Ok(line) = line_io {
            if line.chars().nth(0).unwrap() == '#' { continue; }
            let cols = line.split('\t').collect::<Vec<&str>>();
            assert_eq!(cols.len(), 4);
            variables.push(HashGrammar { 
                name : cols[0].parse().unwrap(),
                size : cols[1].parse().unwrap(),
                left : cols[2].parse().unwrap(),
                right : cols[3].parse().unwrap()
            });
            hash2index.insert(cols[0].parse::<u64>().unwrap(), (variables.len()-1) as u64);
        }
    }

    for var in variables {
        let mut hashwriter = |hash| -> () {
            if hash == u64::MAX {
                writer.write_u32::<LittleEndian>(0).unwrap();
                // eprintln!("{}", 0);
            } else {
                let index_val = *hash2index.get(&hash).unwrap()+256;
                assert_lt!(index_val, u32::MAX as u64);
                writer.write_u32::<LittleEndian>(index_val as u32).unwrap();
                // eprintln!("{}", index_val);
            }
        };
        hashwriter(var.left);
        hashwriter(var.right);
    }
}
