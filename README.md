# **Performance Analysis of Maximal Clique Detection Algorithms**  

## **Overview**  
This project analyzes and compares different maximal clique detection algorithms, including Tomita, Bron-Kerbosch, and Arboricity-based methods. The study aims to evaluate their efficiency on large-scale graph datasets.  

## **Execution Instructions**  

1. **Compile the Algorithm**  
   Run the following command, replacing `<filename>` with the name of the algorithm file and `<output_filename>` with your desired executable name:  

   ```bash
   g++ -O3 <filename>.cpp -o <output_filename>.out
   ```  

   *Note:* The `-O3` flag ensures maximum optimization in the generated executable.  

2. **Run the Compiled Program**  
   ```bash
   ./<output_filename>.out
   ```  

   When prompted, enter the file path of your input graph file. Ensure the file path is correct and relative to the directory in which the algorithm codes are stored to avoid segmentation faults or file not found errors.  

3. **Expected Output Messages**  
   During execution, the program may print status updates before displaying the final results, such as:  

   ```
   Graph Loaded: 103689 edges and 8298 vertices.
   Running Optimized Arboricity-based Clique Algorithm...
   ```  

   These messages help track the execution progress.  

## **Dataset Preparation**  

- Convert directed edges to undirected edges.  
- Scale node indices down to range from `0` to `n-1`.  
- Ensure the dataset is formatted correctly before execution.  

## **Contributors**  

- **Yash Mohta** – Implemented the Tomita and Bron-Kerbosch algorithms.  
- **Utkarsh Bhaskar** – Implemented the Tomita and Bron-Kerbosch algorithms.  
- **Shreeja Basappa** – Implemented the Arboricity-based algorithm, co-authored the project report.  
- **Nidhi Singh** – Implemented the Arboricity-based algorithm, co-authored the project report.  
- **Sidhant Bhat** – Co-authored the project report, developed the project website.  

## **Project Links**  

- **Project Webpage:** [DAA Group 3](https://sidhantbhat3028.github.io/DAA-Group3/)  
- **GitHub Repository:** [DAA Group 3 on GitHub](https://github.com/sidhantbhat3028/DAA-Group3)  
