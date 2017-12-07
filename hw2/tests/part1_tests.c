#include <stdbool.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "tests_macros.h"
#include "hw2_syscalls.h"

static int numbers[1000] = {
        619, 798, 629, 571, 778, 950, 945, 272, 659, 899, 745, 297, 672, 865, 955, 821, 220, 692, 946, 308, 203, 652, 212, 418, 289, 890, 786, 1, 767, 555, 197, 694, 242, 352, 390, 56, 569, 101, 760, 676, 576, 69, 825, 818, 162, 636, 931, 169, 866, 928, 510, 782, 309, 704, 863, 174, 51, 508, 215, 192, 435, 417, 379, 900, 946, 307, 300, 464, 17, 635, 716, 71, 736, 886, 722, 646, 256, 301, 185, 863, 174, 186, 111, 158, 976, 85, 205, 797, 299, 512, 374, 992, 595, 7, 593, 290, 878, 311, 134, 757, 19, 588, 876, 235, 215, 743, 680, 442, 604, 870, 227, 522, 868, 364, 246, 299, 830, 845, 78, 446, 889, 633, 286, 987, 880, 275, 341, 233, 749, 52, 263, 317, 755, 191, 243, 239, 184, 418, 780, 833, 772, 948, 530, 147, 852, 572, 738, 616, 431, 22, 307, 470, 29, 741, 11, 22, 857, 698, 597, 54, 638, 886, 513, 744, 971, 884, 775, 836, 193, 333, 617, 655, 913, 383, 236, 526, 538, 247, 821, 3, 512, 226, 880, 645, 261, 682, 298, 509, 391, 850, 634, 282, 529, 607, 342, 420, 73, 182, 883, 562, 211, 962, 88, 769, 478, 216, 176, 29, 539, 220, 706, 637, 4, 931, 255, 788, 122, 319, 82, 284, 867, 555, 202, 48, 392, 563, 893, 956, 815, 266, 144, 849, 148, 11, 997, 985, 880, 9, 281, 302, 97, 623, 182, 529, 11, 988, 70, 398, 296, 476, 785, 824, 702, 79, 727, 206, 641, 117, 59, 277, 126, 222, 97, 938, 672, 236, 442, 45, 95, 768, 379, 107, 459, 862, 991, 81, 22, 543, 126, 981, 835, 671, 641, 63, 630, 512, 990, 911, 590, 677, 492, 586, 491, 462, 564, 286, 63, 223, 520, 312, 344, 217, 61, 290, 203, 292, 646, 849, 641, 393, 442, 231, 268, 89, 502, 765, 652, 194, 382, 39, 234, 812, 535, 476, 434, 111, 15, 690, 983, 858, 844, 464, 181, 673, 601, 760, 899, 96, 473, 15, 830, 120, 188, 852, 962, 724, 183, 700, 650, 350, 280, 314, 40, 766, 854, 290, 688, 100, 305, 445, 187, 441, 512, 728, 805, 7, 185, 796, 10, 835, 117, 858, 412, 653, 504, 784, 657, 575, 57, 123, 377, 12, 366, 541, 318, 714, 895, 998, 309, 132, 566, 419, 527, 371, 112, 373, 245, 592, 270, 858, 925, 500, 331, 205, 717, 200, 782, 380, 412, 204, 414, 342, 411, 476, 271, 815, 615, 614, 668, 666, 549, 818, 631, 215, 36, 451, 790, 680, 944, 111, 354, 197, 490, 210, 155, 353, 493, 503, 11, 601, 202, 234, 258, 524, 661, 122, 247, 961, 888, 210, 394, 231, 732, 666, 397, 425, 558, 615, 187, 910, 751, 892, 575, 953, 602, 882, 593, 84, 541, 409, 924, 498, 782, 995, 342, 726, 367, 530, 46, 617, 548, 486, 963, 623, 999, 391, 694, 642, 752, 469, 324, 187, 808, 437, 46, 801, 327, 907, 725, 732, 234, 585, 279, 683, 965, 216, 456, 128, 241, 500, 796, 920, 672, 102, 23, 580, 458, 826, 122, 340, 761, 908, 895, 373, 389, 802, 890, 509, 487, 193, 970, 405, 8, 365, 685, 481, 153, 392, 758, 191, 64, 775, 534, 262, 401, 919, 947, 644, 805, 806, 793, 4, 218, 961, 814, 232, 121, 876, 274, 736, 737, 783, 956, 152, 883, 272, 653, 405, 767, 843, 48, 798, 627, 397, 558, 61, 590, 724, 759, 868, 612, 700, 156, 336, 294, 454, 847, 885, 55, 852, 362, 582, 44, 70, 371, 107, 477, 980, 697, 39, 20, 601, 690, 765, 870, 803, 8, 31, 503, 45, 359, 97, 421, 445, 193, 157, 246, 80, 255, 207, 315, 688, 355, 513, 26, 102, 803, 715, 667, 227, 398, 650, 551, 272, 224, 168, 0, 976, 325, 549, 553, 580, 147, 602, 480, 798, 34, 229, 989, 486, 465, 183, 773, 949, 442, 145, 372, 530, 72, 159, 373, 533, 577, 781, 959, 357, 908, 530, 67, 262, 816, 369, 688, 714, 837, 411, 338, 942, 1, 376, 445, 247, 506, 130, 567, 965, 804, 23, 586, 206, 690, 854, 726, 41, 579, 915, 646, 939, 240, 163, 788, 431, 424, 752, 798, 774, 542, 977, 838, 134, 542, 229, 471, 488, 268, 89, 715, 115, 994, 374, 863, 414, 1, 252, 440, 30, 794, 906, 40, 470, 605, 483, 503, 831, 640, 617, 950, 296, 238, 53, 272, 657, 415, 702, 674, 254, 418, 140, 711, 190, 119, 207, 695, 511, 503, 992, 494, 636, 330, 102, 690, 147, 445, 767, 244, 995, 134, 425, 718, 67, 374, 556, 511, 405, 451, 716, 368, 730, 214, 82, 90, 223, 610, 511, 841, 154, 426, 831, 108, 430, 202, 759, 39, 82, 193, 110, 535, 947, 571, 400, 339, 655, 365, 32, 360, 904, 124, 680, 996, 904, 634, 161, 910, 974, 418, 936, 890, 424, 683, 108, 371, 123, 44, 259, 17, 222, 268, 184, 691, 718, 290, 912, 357, 471, 957, 429, 453, 656, 169, 422, 175, 210, 198, 622, 595, 239, 247, 350, 364, 847, 607, 570, 734, 884, 841, 71, 948, 93, 162, 361, 29, 115, 755, 386, 619, 226, 474, 108, 647, 649, 657, 747, 240, 88, 673, 54, 812, 853, 690, 1, 309, 544, 678, 812, 137, 424, 733, 620, 365, 154, 484, 322, 597, 283, 203, 797, 596, 546, 587, 76, 320, 855, 94, 43, 444, 855, 702, 821, 618, 957, 409, 904, 461, 53, 907, 479, 388, 614, 470, 402, 246, 307, 310, 426, 852, 803, 719, 447, 419, 565, 264, 336, 31, 560, 422, 51, 623, 917, 317, 873, 475, 264, 745, 226, 640, 906, 129, 818, 674, 640, 652, 952, 697, 63, 964, 592, 649, 842, 424, 394, 349, 558, 465, 233, 866, 991, 481, 333, 16, 214, 408, 20, 791, 397, 816, 376, 787, 462, 300, 89, 679, 847, 554, 488, 865, 892, 871, 720, 829, 960, 358, 312, 700, 888, 109, 836, 904, 216, 920, 525
};

int intcmp(const void *a, const void *b)
{
    if (*(int*)a < *(int*)b)
        return -1;
    else if (*(int*)a == *(int*)b)
        return 0;
    else
        return 1;
}

static int timeslices_by_prio[140] =
        { 532, 528, 524, 521, 517, 513, 509, 505, 502, 498,
          494, 490, 486, 483, 479, 475, 471, 467, 464, 460,
          456, 452, 449, 445, 441, 437, 433, 430, 426, 422,
          418, 414, 411, 407, 403, 399, 395, 392, 388, 384,
          380, 376, 373, 369, 365, 361, 357, 354, 350, 346,
          342, 338, 335, 331, 327, 323, 319, 316, 312, 308,
          304, 301, 297, 293, 289, 285, 282, 278, 274, 270,
          266, 263, 259, 255, 251, 247, 244, 240, 236, 232,
          228, 225, 221, 217, 213, 209, 206, 202, 198, 194,
          190, 187, 183, 179, 175, 171, 168, 164, 160, 156,
          153, 149, 145, 141, 137, 134, 130, 126, 122, 118,
          115, 111, 107, 103, 99, 96, 92, 88, 84, 80,
          77, 73, 69, 65, 61, 58, 54, 50, 46, 42,
          39, 35, 31, 27, 23, 20, 16, 12, 8, 5 };

int get_remaining_timeslice_invalid_pid()
{
    int res = get_remaining_timeslice(-1);
    ASSERT_EQUALS(res, -1);
    ASSERT_EQUALS(errno, ESRCH);
    return 1;
}

int get_remaining_timeslice_success()
{
    pid_t father = getpid();
    pid_t children[40] = { 0 };
    int results[40] = { 0 };
    int i;
    for (i = 19; i >= -20; --i)
    {
        children[i + 20] = fork();
        if (!children[i + 20]) /* child process */
        {
            nice(i);

            ASSERT_NOT_NEGATIVE(sacrifice_timeslice(father)); /* zero the timeslice to get a fresh one */

            pid_t childpid = getpid();
            int j, maxval = timeslices_by_prio[120 + i];
            int res;
            for (j = 0; j < 1000; ++j)
            {
                res = get_remaining_timeslice(childpid);
                if (res == -1)
                {
                    printf("syscall failed\n");
                    exit(-1);
                } /* syscall failed */
                if (res > maxval)
                {
                    printf("unexpected timeslice > maxval: %d > %d\n", res, maxval);
                    exit(-2);
                } /* unexpected timeslice */
            }
            exit(0); /* success */
        }
        else /* father process */
        {
            waitpid(children[i + 20], results + i + 20, 0);
            if (results[i + 20])
                printf("process %d with prio %d failed with %d\n", children[i + 20], 120+i, results[i + 20]);
            ASSERT_EQ(results[i + 20], 0);
        }
    }
    return 1;
}

int get_remaining_timeslice_fifo()
{
    struct sched_param p;
    p.sched_priority = 1;
    pid_t pid = getpid();
    sched_setscheduler(pid, SCHED_FIFO, &p); /* set current task policy to SCHED_FIFO with priority of 1 */
    int res = get_remaining_timeslice(pid);
    p.sched_priority = 0;
    sched_setscheduler(pid, SCHED_OTHER, &p); /* return to SCHED_OTHER */
    ASSERT_EQUALS(res, -1);
    ASSERT_EQUALS(errno, EINVAL);
    return 1;
}

int get_total_processor_usage_invalid_pid()
{
    int res = get_total_processor_usage(-1);
    ASSERT_EQUALS(res, -1);
    ASSERT_EQUALS(errno, ESRCH);
    return 1;
}

int get_total_processor_usage_zombie()
{
    pid_t child = fork();
    if (!child) /* child code */
    {
        int i;
        for (i = 0; i < 1000; ++i)
        {
            qsort(numbers, 1000, sizeof(int), intcmp); /* do some work */
        }

        exit(0);
    }
    else /* parent code */
    {
        sleep(10); /* make sure the child finish first */
        int child_usage = get_total_processor_usage(child); /* child should now be a zombie */
        int parent_usage = get_total_processor_usage(getpid());
        ASSERT_NEQ(child_usage, -1);
        ASSERT_NEQ(parent_usage, -1);
        ASSERT_LE(parent_usage, child_usage);
        printf("\nParent CPU time: %d ticks, Child CPU time: %d ticks. ", parent_usage, child_usage);
        return 1;
    }
}

int get_total_processor_usage_child_zero()
{
    int i;
    for (i = 0; i < 1000; ++i)
    {
        qsort(numbers, 1000, sizeof(int), intcmp); /* do some work */
    }
    pid_t child = fork();

    if (!child) /* child code */
    {
        exit(get_total_processor_usage(getpid()));
    }
    else /* parent code */
    {
        int child_usage = 0;
        waitpid(child, &child_usage, 0);
        int parent_usage = get_total_processor_usage(getpid());
        ASSERT_NEQ(child_usage, -1);
        ASSERT_NEQ(parent_usage, -1);
        ASSERT_GE(parent_usage, child_usage);
        printf("\nParent CPU time: %d ticks, Child CPU time: %d ticks. ", parent_usage, child_usage);
        return 1;
    }
}

int get_total_time_in_runqueue_invalid_pid()
{
    int res = get_total_time_in_runqueue(-1);
    ASSERT_EQUALS(res, -1);
    ASSERT_EQUALS(errno, ESRCH);
    return 1;
}

int get_total_time_in_runqueue_functionallity()
{
    /* do everything in a new process */
    if (fork()) exit(0);

    nice(19);
    pid_t pid = getpid();
    pid_t children[10] = { 0 };
    int cpu_times[10] = { 0 };
    int rq_times[10] = { 0 };
    int i;
    for (i = 0; i < 10; ++i)
    {
        children[i] = fork();
        if (!children[i]) /* child code */
        {
            nice(-20);
            for (i = 0; i < 1000; ++i)
            {
                qsort(numbers, 1000, sizeof(int), intcmp); /* do some work */
            }
            exit(0);
        }
    }
    int sum_of_rq_times = 0;
    for (i = 0; i < 1000; ++i)
        sched_yield(); /* make sure children finish first */
    for (i = 0; i < 10; ++i)
    {
        cpu_times[i] = get_total_processor_usage(children[i]);
        ASSERT_NOT_NEGATIVE(cpu_times[i]);
        rq_times[i] = get_total_time_in_runqueue(children[i]);
        ASSERT_NOT_NEGATIVE(rq_times[i]);
        sum_of_rq_times += rq_times[i];
        ASSERT_LE(cpu_times[i], rq_times[i]);
    }

    for (i = 0; i < 10; ++i)
        wait(NULL);

    int parent_cpu_time = get_total_processor_usage(pid);
    int parent_rq_time = get_total_time_in_runqueue(pid);

    ASSERT_GE(parent_rq_time, parent_cpu_time);

    for (i = 0; i < 10; ++i)
    {
        ASSERT_GE(parent_rq_time, rq_times[i]);
        ASSERT_LE(parent_cpu_time, cpu_times[i]);
    }

    return 1;
}

int sacrifice_timeslice_invalid_pid()
{
    int res = sacrifice_timeslice(-1);
    ASSERT_EQUALS(res, -1);
    ASSERT_EQUALS(errno, ESRCH);
    return 1;
}

int sacrifice_timeslice_same_pid()
{
    int res = sacrifice_timeslice(getpid());
    ASSERT_EQUALS(res, -1);
    ASSERT_EQUALS(errno, EINVAL);
    return 1;
}

int sacrifice_timeslice_to_fifo()
{
    pid_t child = fork();
    if (!child) /* child code */
    {
        struct sched_param p;
        p.sched_priority = 1;
        pid_t pid = getpid();
        sched_setscheduler(pid, SCHED_FIFO, &p); /* set current task policy to SCHED_FIFO with priority of 1 */
        sleep(10);
        exit(0);
    }

    sleep(5);
    int res = sacrifice_timeslice(child);
    ASSERT_EQUALS(res, -1);
    ASSERT_EQUALS(errno, EINVAL);
    wait(NULL);
    return 1;
}

int sacrifice_timeslice_from_fifo()
{
    pid_t parent = getpid();
    struct sched_param p;
    p.sched_priority = 0;
    sched_setscheduler(parent, SCHED_OTHER, &p); /* make sure parent is SCHED_OTHER */

    pid_t child = fork();
    if (!child) /* child code */
    {
        p.sched_priority = 1;
        pid_t pid = getpid();
        sched_setscheduler(pid, SCHED_FIFO, &p); /* set current task policy to SCHED_FIFO with priority of 1 */
        int res = sacrifice_timeslice(parent);
        ASSERT_EQUALS(res, -1);
        ASSERT_EQUALS(errno, EPERM);
        return 1;
    }
    wait(NULL);
    exit(0);
}

int sacrifice_timeslice_to_zombie()
{
    pid_t parent = getpid();
    struct sched_param p;
    p.sched_priority = 0;
    sched_setscheduler(parent, SCHED_OTHER, &p); /* make sure parent is SCHED_OTHER */

    pid_t child = fork();
    if (!child) /* child code */
        exit(0);

    int i;
    for (i = 0; i < 1000; ++i)
        sched_yield(); /* make sure the child dies first */

    int res = sacrifice_timeslice(child);
    ASSERT_EQUALS(res, -1);
    wait(NULL);
    return 1;
}

int main()
{

    RUN_TEST(get_remaining_timeslice_invalid_pid);
    RUN_TEST(get_remaining_timeslice_success);
    RUN_TEST(get_remaining_timeslice_fifo);
    RUN_TEST(get_total_processor_usage_invalid_pid);
    RUN_TEST(get_total_processor_usage_zombie);
    RUN_TEST(get_total_processor_usage_child_zero);
    RUN_TEST(get_total_time_in_runqueue_invalid_pid);
    RUN_TEST(get_total_time_in_runqueue_functionallity);
    RUN_TEST(sacrifice_timeslice_invalid_pid);
    RUN_TEST(sacrifice_timeslice_same_pid);
    RUN_TEST(sacrifice_timeslice_to_fifo);
    RUN_TEST(sacrifice_timeslice_from_fifo);
    RUN_TEST(sacrifice_timeslice_to_zombie);

    printf("All tests passed :)\n");
    return 0;
}
