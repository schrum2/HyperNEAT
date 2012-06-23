"""
##############################################################################
# plot_rewards.py
#
# This is similar to plot_exported_vector.py, except that it is specifically
# designed for generating the paper/thesis plots
##############################################################################
"""
import  sys, pylab
from    sys     import  exit
from    pylab   import  *
from    scipy   import  *
from    os      import  system
from    retrieve_and_summarize_results  \
                import  load_best_action_summary, load_vector

def plot_rewards_per_episode(file, agent_name, game_name, delimiter = ',', num_steps = 1):
    """
        Plots and saves the exported rewarde-per-episode vector
        num_steps determines the walking average steps.
    """
    new_file = ungzip(file)
    if new_file is not None:
        file = new_file
    fin = open(file)
    inline = fin.readline()
    fin.close()
    inline_split = inline.split(delimiter)
    x = []
    for val in inline_split:
        if val.strip() != "":
            x.append(float(val))
    walking_avg_x = get_moving_average(x, num_steps)
    
    # Get the random and best-action results
    action_summary = load_best_action_summary(game_name, len(walking_avg_x))
    rand_rew = action_summary[game_name]['random_rew']
    best_act_rew = action_summary[game_name]['best_act_rew']
    zeros_vec = zeros(len(walking_avg_x)) 
    rand_vec = zeros_vec + rand_rew
    best_act_vec = zeros_vec + best_act_rew
    max_val = max(max(walking_avg_x), rand_rew, best_act_rew)
    min_val = min(min(walking_avg_x), rand_rew, best_act_rew)
    
    F = pylab.gcf()
    DefaultSize = F.get_size_inches()
    F.set_size_inches( (DefaultSize[0]*0.75, DefaultSize[1]) )
    params = {  'backend': 'ps',
                'axes.labelsize': 14,
                'text.fontsize': 14,
                'legend.fontsize': 14,
                'xtick.labelsize': 14,
                'ytick.labelsize': 14,
                'text.usetex': True}
    pylab.rcParams.update(params)

    xlabel("Episode Number")
    ylabel("Reward per Episode")
    hold(True)
    plot(walking_avg_x, label=agent_name)
    plot(rand_vec,      linestyle = '--', label = "Random Average Reward")
    plot(best_act_vec,  linestyle = '-.', label = "Best-Action  Average Reward")
    plot((0, max_val * 1.1))
    plot((0, min_val - 0.1))
    hold(False)
    legend()
    
    filname = "rl_" + agent_name.replace("-", "_").lower() + "_" + \
                game_name.replace("-", "_").lower() + "_rew_per_epis"
    filname += ".png"
    savefig(filname)
    if new_file is not None:
        gzip(file)

    
def plot_rewards_per_frame(file, agent_name, game_name, delimiter, num_steps):
    """
        Plots and saves the exported rewarde-per-frame vector
        num_steps determines how many vectpr values are averaged
        for each point on the plot
    """
    new_file = ungzip(file)
    if new_file is not None:
        file = new_file
    fin = open(file)
    inline = fin.readline()
    fin.close()
    inline_split = inline.split(delimiter)
    x = []
    for val in inline_split:
        if val.strip() != "":
            x.append(float(val))
    plot_v = []
    for i in range(len(x) / num_steps):
        start_ind = i * num_steps
        end_ind = (i+1) * num_steps
        plot_v.append(float(sum(x[start_ind:end_ind])) / float(num_steps))
    
    xlabel("Frame Number (x %d)"%num_steps)
    ylabel("Reward per Frame (averaged over %d frames)" % num_steps)
    plot(plot_v)
    filname = file[0:file.rfind('.')]
    if num_steps != 1:
        filname += "_a_%d_frames"%num_steps
    filname += ".png"
    savefig(filname)
    if new_file is not None:
        gzip(file)


def get_moving_average(vec, num_steps):
    if num_steps == 1:
        return vec
    moving_average_vec = []
    for i in range(num_steps, len(vec) - num_steps):
        sum = 0
        sum += vec[i]
        for j in range(1, num_steps + 1):
            sum += vec[i - j]
            sum += vec[i + j]
        average = float(sum) / float(2 * num_steps + 1)
        moving_average_vec.append(average)
    return moving_average_vec
    
def ungzip(file):
    "if the file is compresses, it uncompresses it returns the enw name"
    if file.find('.gz') > 0:
        system("gunzip %s" % file)
        file = file[:file.rfind('.gz')]
        return file
    return None

def gzip(file):
    "re-comprsses the given file"
    system("gzip %s" % file)

def get_random_bestact_perf(game_name, vec_len):
    print "game_name", game_name, "vec_len", vec_len
    # Get the random and best-action results
    action_summary = load_best_action_summary()
    rand_rew = action_summary[game_name]['random_rew']
    best_act_rew = action_summary[game_name]['best_act_rew']
    zeros_vec = zeros(vec_len) 
    rand_vec = zeros_vec + rand_rew
    best_act_vec = zeros_vec + best_act_rew
    return rand_rew, rand_vec, best_act_rew, best_act_vec


if __name__ == "__main__":
    err_str =   "Usage: \n" + \
                "\t python plot_rewards_per_episode.py agent_name game_name per_epis/per_frame file " + \
                "[Moving Average Steps]"

    arg_len = len(sys.argv)
    if arg_len != 6:
        print err_str
        exit(-1)
    agent_name = sys.argv[1]
    game_name = sys.argv[2]
    num_steps = int(sys.argv[5])
    file = sys.argv[4]
    if sys.argv[3] == "per_epis":
        plot_rewards_per_episode(file, agent_name, game_name, ',', num_steps)
    elif sys.argv[3] == "per_frame":
        plot_rewards_per_frame(file, agent_name, game_name,',', num_steps)
    else:
        print err_str


