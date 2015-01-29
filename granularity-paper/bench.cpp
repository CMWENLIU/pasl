/* COPYRIGHT (c) 2014 Umut Acar, Arthur Chargueraud, and Michael
 * Rainey
 * All rights reserved.
 *
 * \file bench.cpp
 * \brief Benchmarking driver
 *
 */

#include "benchmark.hpp"
#include "hash.hpp"
#include "dup.hpp"
#include "string.hpp"
#include "sort.hpp"
#include "graph-lite.hpp"
#include "fib.hpp"
#include "mcss.hpp"
#include "numeric.hpp"
#include "exercises.hpp"
#include "nearestneighbors-lite.hpp"
#include "synthetic-lite.hpp"

/***********************************************************************/

loop_controller_type almost_sorted_sparray_contr("almost_sorted_sparray");

// returns an array that is sorted up to a given number of swaps
sparray almost_sorted_sparray(long s, long n, long nb_swaps) {
  sparray tmp = sparray(n);
  par::parallel_for(almost_sorted_sparray_contr, 0l, n, [&] (long i) {
    tmp[i] = (value_type)i;
  });
  for (long i = 0; i < nb_swaps; i++)
    std::swap(tmp[random_index(2*i, n)], tmp[random_index(2*i+1, n)]);
  return tmp;
}

loop_controller_type exp_dist_sparray_contr("exp_dist_sparray");

// returns an array with exponential distribution of size n using seed s
sparray exp_dist_sparray(long s, long n) {
  sparray tmp = sparray(n);
  int lg = log2_up(n)+1;
  par::parallel_for(exp_dist_sparray_contr, 0l, n, [&] (long i) {
    long range = (1 << (random_index(2*(i+s), lg)));
    tmp[i] = (value_type)hash64shift((long)(range+random_index(2*(i+s), range)));
  });
  return tmp;
}

/*---------------------------------------------------------------------*/
/* Benchmark framework */

using thunk_type = std::function<void ()>;

using benchmark_type =
  std::pair<std::pair<thunk_type,thunk_type>,
            std::pair<thunk_type, thunk_type>>;

benchmark_type make_benchmark(thunk_type init, thunk_type bench,
                              thunk_type output, thunk_type destroy) {
  return std::make_pair(std::make_pair(init, bench),
                        std::make_pair(output, destroy));
}

void bench_init(const benchmark_type& b) {
  b.first.first();
}

void bench_run(const benchmark_type& b) {
  b.first.second();
}

void bench_output(const benchmark_type& b) {
  b.second.first();
}

void bench_destroy(const benchmark_type& b) {
  b.second.second();
}

/*---------------------------------------------------------------------*/
/* Benchmark definitions */

benchmark_type fib_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 38);
  long* result = new long;
  auto init = [&] {

  };
  auto bench = [=] {
    fib_contr.initialize(1, 10);
//    std::cerr << "Here!" << std::endl;
    *result = fib(n);
  };
  auto output = [=] {
    std::cout << "result " << *result << std::endl;
  };
  auto destroy = [=] {
    delete result;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type mfib_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 38);
  long* result = new long;
  auto init = [=] {
    
  };
  auto bench = [=] {
    *result = mfib(n);
  };
  auto output = [=] {
    std::cout << "result " << *result << std::endl;
  };
  auto destroy = [=] {
    delete result;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type map_incr_bench(bool student_soln = false) {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp = new sparray(0);
  sparray* outp = new sparray(0);
  auto init = [=] {
    *inp = fill(n, 1);
  };
  auto bench = [=] {
    sparray& in = *inp;
    if (student_soln) {
      *outp = sparray(in.size());
      exercises::map_incr(&in[0], &(*outp)[0], in.size());
    } else {
      *outp = map([&] (value_type x) { return x+1; }, in);
    }
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type duplicate_bench(bool ex = false) {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp = new sparray(0);
  sparray* outp = new sparray(0);
  auto init = [=] {
    *inp = fill(n, 1);
  };
  auto bench = [=] {
    *outp = (ex) ? exercises::duplicate(*inp) : duplicate(*inp);
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type ktimes_bench(bool ex = false) {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  long k = pasl::util::cmdline::parse_or_default_long("k", 4);
  sparray* inp = new sparray(0);
  sparray* outp = new sparray(0);
  auto init = [=] {
    *inp = fill(n, 1);
  };
  auto bench = [=] {
    *outp = (ex) ? exercises::ktimes(*inp, k) : ktimes(*inp, k);
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

using reduce_bench_type = enum { reduce_normal, reduce_max_ex, reduce_plus_ex, reduce_ex };

benchmark_type reduce_bench(reduce_bench_type t = reduce_normal) {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp = new sparray(0);
  value_type* result = new value_type;
  auto init = [=] {
    *inp = fill(n, 1);
  };
  auto bench = [=] {
    if (t == reduce_normal)
      *result = sum(*inp);
    else if (t == reduce_max_ex)
      *result = exercises::max(&(*inp)[0], inp->size());
    else if (t == reduce_plus_ex)
      *result = exercises::plus(&(*inp)[0], inp->size());
    else if (t == reduce_ex)
      *result = exercises::reduce(plus_fct, 0l, &(*inp)[0], inp->size());
  };
  auto output = [=] {
    std::cout << "result " << *result << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete result;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type scan_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp = new sparray(0);
  sparray* outp = new sparray(0);
  auto init = [=] {
    *inp = fill(n, 1);
  };
  auto bench = [=] {
    *outp = prefix_sums_excl(*inp).partials;
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type filter_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp = new sparray(0);
  sparray* outp = new sparray(0);
  auto init = [=] {
    *inp = gen_random_sparray(n);
  };
  auto bench = [=] {
    *outp = exercises::filter(is_even_fct, *inp);
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type mcss_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp = new sparray(0);
  value_type* outp = new value_type;
  auto init = [=] {
    *inp = gen_random_sparray(n);
  };
  auto bench = [=] {
    *outp = mcss(*inp);
  };
  auto output = [=] {
    std::cout << "result " << *outp << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type dmdvmult_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 4000);
  long nxn = n*n;
  sparray* mtxp = new sparray(0);
  sparray* vecp = new sparray(0);
  sparray* outp = new sparray(0);
  auto init = [=] {
    *mtxp = gen_random_sparray(nxn);
    *vecp = gen_random_sparray(n);
  };
  auto bench = [=] {
    *outp = dmdvmult(*mtxp, *vecp);
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete mtxp;
    delete vecp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type merge_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp1 = new sparray(0);
  sparray* inp2 = new sparray(0);
  sparray* outp = new sparray(0);
  pasl::util::cmdline::argmap<std::function<sparray (sparray&,sparray&)>> algos;
  algos.add("ours", [] (sparray& xs, sparray& ys) { return merge(xs, ys); });
  algos.add("cilk", [] (sparray& xs, sparray& ys) { return cilkmerge(xs, ys); });
  auto merge_fct = algos.find_by_arg("algo");
  auto init = [=] {
    pasl::util::cmdline::argmap_dispatch c;
    *inp1 = gen_random_sparray(n);
    *inp2 = gen_random_sparray(n);
    in_place_sort(*inp1);
    in_place_sort(*inp2);
  };
  auto bench = [=] {
    *outp = merge_fct(*inp1, *inp2);
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete inp1;
    delete inp2;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type sort_bench() {
  long n = pasl::util::cmdline::parse_or_default_long("n", 1l<<20);
  sparray* inp = new sparray(0);
  sparray* outp = new sparray(0);
  pasl::util::cmdline::argmap<std::function<sparray (sparray&)>> algos;
  algos.add("quicksort",          [] (sparray& xs) { return quicksort(xs); });
  algos.add("mergesort",          [] (sparray& xs) { return mergesort(xs); });
  algos.add("mergesort_seqmerge", [] (sparray& xs) { return mergesort<false>(xs); });
  algos.add("cilksort",           [] (sparray& xs) { return cilksort(xs); });
//  algos.add("bmssort",            [] (sparray& xs) { return bms_sort(xs); });
  algos.add("mergesort_ex",       [] (sparray& xs) { return mergesort_ex(xs); });
                           
  std::string bench_sort = pasl::util::cmdline::parse_or_default_string(
        "bench", std::string("bmssort"));
                                                                         
  if (bench_sort == "bmssort") {
    pasl::util::cmdline::argmap<std::function<sparray (sparray&)>> block;
    block.add("log2n", [] (sparray& xs) { return bms_sort_log2n(xs); });
    block.add("sqrtn", [] (sparray& xs) { return bms_sort_sqrtn(xs); });
    block.add("n",     [] (sparray& xs) { return bms_sort_n(xs); });
    algos.add("bmssort", block.find_by_arg("block"));
  }

  auto sort_fct = algos.find_by_arg("bench");
  auto init = [=] {
    pasl::util::cmdline::argmap_dispatch c;
    c.add("random", [=] {
      *inp = gen_random_sparray(n);
    });
    c.add("almost_sorted", [=] {
      long nb_swaps = pasl::util::cmdline::parse_or_default_long("nb_swaps", 1000);
      *inp = almost_sorted_sparray(1232, n, nb_swaps);
    });
    c.add("exponential_dist", [=] {
      *inp = exp_dist_sparray(12323, n);
    });
    c.find_by_arg_or_default_key("generator", "random")();

//    bms_sort_contr.initialize(1, 10);

    std::string running_mode = pasl::util::cmdline::parse_or_default_string(
          "mode", std::string("by_force_sequential"));

    #ifdef CMDLINE
      std::cout << "Using " << running_mode << " mode" << std::endl;
    #elif PREDICTION
      std::cout << "Using by_prediction mode" << std::endl;
    #elif CUTOFF_WITH_REPORTING
      std::cout << "Using by_cutoff_with_reporting mode" << std::endl;
    #elif CUTOFF_WITHOUT_REPORTING        
      std::cout << "Using by_cutoff_without_reporting mode" << std::endl;
    #endif
    bms_memcpy_contr.set(running_mode);
    bms_merge_contr.set(running_mode);
    bms_sort_contr.set(running_mode);
  };
  auto bench = [=] {
    *outp = sort_fct(*inp);
  };
  auto output = [=] {
    std::cout << "result " << (*outp)[outp->size()-1] << std::endl;
  };
  auto destroy = [=] {
    delete inp;
    delete outp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type graph_bench() {
  adjlist* graphp = new adjlist;
  sparray* distsp = new sparray;
  std::string fname = pasl::util::cmdline::parse_or_default_string("fname", "");
  vtxid_type source = pasl::util::cmdline::parse_or_default_long("source", (value_type)0);
  if (fname == "")
    pasl::util::atomic::fatal([] { std::cerr << "missing filename for graph: -fname filename"; });
  auto init = [=] {
    graphp->load_from_file(fname);
  };
  auto bench = [=] {
    *distsp = bfs(*graphp, source);
  };
  auto output = [=] {                                                      
    long nb_visited = sum(map([] (value_type v) { return (v != 0); }, *distsp));
    long max_dist = max(*distsp);
    std::cout << "nb_visited\t" << nb_visited << std::endl;
    std::cout << "max_dist\t" << max_dist << std::endl;
  };
  auto destroy = [=] {
    delete graphp;
    delete distsp;
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type nearestneighbors_bench() {            
  AbstractRunnerNN** runner_link = new AbstractRunnerNN*;
                                                                                            
  auto init = [=] {             
    int n = pasl::util::cmdline::parse_or_default_int("n", 1000000);
    int k = pasl::util::cmdline::parse_or_default_int("k", 8);
    int d = pasl::util::cmdline::parse_or_default_int("d", 2);
    std::string gen_type = pasl::util::cmdline::parse_or_default_string(
        "gen", "uniform");
    bool inSphere = pasl::util::cmdline::parse_or_default_bool(
        "in-sphere", false);
    bool onSphere = pasl::util::cmdline::parse_or_default_bool(
        "on-sphere", false);
                             
    AbstractRunnerNN* runner = NULL;

    if (d == 2) {
      if (gen_type.compare("uniform") == 0) {
        pbbs::point2d* points = pbbs::uniform2d(inSphere, onSphere, n);
        runner = new RunnerNN<int, pbbs::point2d, 20>(preparePoints<pbbs::point2d, 20>(n, points), n, k);
      } else if (gen_type.compare("plummer") == 0) {        pbbs::point2d* points = pbbs::plummer2d(n);
        runner = new RunnerNN<int, pbbs::point2d, 20>(preparePoints<pbbs::point2d, 20>(n, points), n, k);
      } else {
        std::cerr << "Wrong generator type " << gen_type << "\n";
        exit(-1);
      }
    } else {
      if (gen_type.compare("uniform") == 0) {
        pbbs::point3d* points = pbbs::uniform3d<int, int>(inSphere, onSphere, n);
        runner = new RunnerNN<int, pbbs::point3d, 20>(preparePoints<pbbs::point3d, 20>(n, points), n, k);
      } else if (gen_type.compare("plummer") == 0) {
        pbbs::point3d* points = pbbs::plummer3d<int, int>(n);
        runner = new RunnerNN<int, pbbs::point3d, 20>(preparePoints<pbbs::point3d, 20>(n, points), n, k);
      } else {
        std::cerr << "Wrong generator type " << gen_type << "\n";
        exit(-1);
      }                 
    }
    *runner_link = runner;
    std::string running_mode = pasl::util::cmdline::parse_or_default_string(
          "mode", std::string("by_force_sequential"));

    #ifdef CMDLINE
      std::cout << "Using " << running_mode << " mode" << std::endl;
    #elif PREDICTION
      std::cout << "Using by_prediction mode" << std::endl;
    #elif CUTOFF_WITH_REPORTING
      std::cout << "Using by_cutoff_with_reporting mode" << std::endl;
    #elif CUTOFF_WITHOUT_REPORTING        
      std::cout << "Using by_cutoff_without_reporting mode" << std::endl;
    #endif

    nn_build_contr.set(running_mode);
    nn_run_contr.set(running_mode);
  };            
                     
  auto bench = [=] {
    std::cerr << "Initialization have started!\n";
    (*runner_link)->initialize();
    std::cerr << "Initialization have finished!\n";
    (*runner_link)->run();
  };
  auto output = [=] {                  
    std::cout << "The evaluation have finished" << std::endl;
  };               
  auto destroy = [=] {
    (*runner_link)->free();
  };
  return make_benchmark(init, bench, output, destroy);
}

benchmark_type synthetic_bench() {
  int n = pasl::util::cmdline::parse_or_default_int(
        "n", 2000);
  int cn = pasl::util::cmdline::parse_or_default_int("c", 10000000);
  int m = pasl::util::cmdline::parse_or_default_int(
        "m", cn / n);
  int p = pasl::util::cmdline::parse_or_default_int(
        "p", 100);                                         

  auto init = [&] {
    std::string running_mode = pasl::util::cmdline::parse_or_default_string(
          "mode", std::string("by_force_sequential")).c_str();

    #ifdef CMDLINE
      std::cout << "Using " << running_mode << " mode" << std::endl;
    #elif PREDICTION
      std::cout << "Using by_prediction mode" << std::endl;
    #elif CUTOFF_WITH_REPORTING
      std::cout << "Using by_cutoff_with_reporting mode" << std::endl;
    #elif CUTOFF_WITHOUT_REPORTING        
      std::cout << "Using by_cutoff_without_reporting mode" << std::endl;
    #endif

    sol_contr.set(running_mode);
    sil_contr.set(running_mode);

    sf_contr.set(running_mode);
    sg_contr.set(running_mode);
  };
                                     
  pasl::util::cmdline::argmap_dispatch c;
  c.add("parallel_for", [=] {
    synthetic(n, m, p);
  });
                        
  c.add("recursive", [=] {
    synthetic_f(n, m, p);
  });
                                       
  auto bench = c.find_by_arg("algo");
                 
  auto output = [=] {                  
    std::cout << "result " << synthetic_total.mine() << std::endl;
  };                  
  auto destroy = [=] {
  };
  return make_benchmark(init, bench, output, destroy);
}

void init_controllers() {
  int tries = pasl::util::cmdline::parse_or_default_int("tries", 10);
  double init_est = pasl::util::cmdline::parse_or_default_int("init", 1)
                                                                        ;
  // Synthetic benchmark controllers
  sol_contr.initialize(init_est, tries);
  sil_contr.initialize(init_est, tries);

  sf_contr.initialize(init_est, tries);
  sg_contr.initialize(init_est, tries);

  // Nearest neighbors benchmark controllers
  nn_build_contr.initialize(init_est, tries);
  nn_run_contr.initialize(init_est, tries);

  // Sort benchmark controllers
  bms_memcpy_contr.initialize(init_est, tries);
  bms_merge_contr.initialize(init_est, tries);
  bms_sort_contr.initialize(init_est, tries);

  quicksort_contr.initialize(init_est, tries);

  merge_contr.initialize(init_est, tries);

  mergesort_contr.initialize(init_est, tries);

  mergesort_ex_contr.initialize(init_est, tries);

  cilkmerge_contr.initialize(init_est, tries);

  cilksort_contr.initialize(init_est, tries);
}

/*---------------------------------------------------------------------*/
/* PASL Driver */

int main(int argc, char** argv) {

  benchmark_type bench;
  
  auto init = [&] {
    pasl::util::cmdline::argmap<std::function<benchmark_type()>> m;
    m.add("fib",                  [&] { return fib_bench(); });
    m.add("mfib",                 [&] { return mfib_bench(); });
    m.add("map_incr",             [&] { return map_incr_bench(); });
    m.add("reduce",               [&] { return reduce_bench(); });
    m.add("scan",                 [&] { return scan_bench(); });
    m.add("mcss",                 [&] { return mcss_bench(); });
    m.add("dmdvmult",             [&] { return dmdvmult_bench(); });
    m.add("merge",                [&] { return merge_bench(); });
    m.add("quicksort",            [&] { return sort_bench(); });
    m.add("mergesort",            [&] { return sort_bench(); });
    m.add("mergesort_seqmerge",   [&] { return sort_bench(); });
    m.add("cilksort",             [&] { return sort_bench(); });
    m.add("bmssort",             [&] { return sort_bench(); });
    m.add("graph",                [&] { return graph_bench(); });
    m.add("duplicate",            [&] { return duplicate_bench(); });
    m.add("ktimes",               [&] { return ktimes_bench(); });
    m.add("nearest_neighbors",    [&] { return nearestneighbors_bench(); });
    m.add("synthetic",            [&] { return synthetic_bench(); });
    

    m.add("map_incr_ex",          [&] { return map_incr_bench(true); });
    m.add("sum_ex",               [&] { return reduce_bench(reduce_plus_ex); });
    m.add("max_ex",               [&] { return reduce_bench(reduce_max_ex); });
    m.add("reduce_ex",            [&] { return reduce_bench(reduce_ex); });
    m.add("duplicate_ex",         [&] { return duplicate_bench(true); });
    m.add("ktimes_ex",            [&] { return ktimes_bench(true); });
    m.add("filter_ex",            [&] { return filter_bench(); });
    m.add("mergesort_ex",         [&] { return sort_bench(); });
    
    bench = m.find_by_arg("bench")();

    init_controllers();

    bench_init(bench);
  };
  auto run = [&] (bool) {
    bench_run(bench);
  };
  auto output = [&] {
    bench_output(bench);
  };
  auto destroy = [&] {
    bench_destroy(bench);
  };
  pasl::sched::launch(argc, argv, init, run, output, destroy);
}

/***********************************************************************/
