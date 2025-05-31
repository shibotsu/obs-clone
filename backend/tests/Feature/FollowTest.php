<?php

namespace Tests\Feature;

use App\Models\User;
use Illuminate\Foundation\Testing\RefreshDatabase;
use Illuminate\Foundation\Testing\WithFaker;
use Laravel\Sanctum\Sanctum;
use Tests\TestCase;
use Tymon\JWTAuth\Facades\JWTAuth;

class FollowTest extends TestCase
{
    use RefreshDatabase;

    public function test_user_can_follow_and_unfollow()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();

        $token = JWTAuth::fromUser($user);

        $this->withHeader('Authorization', "Bearer $token")
            ->postJson("/api/follow/{$target->id}")
            ->assertStatus(200);

        $this->assertDatabaseHas('follows', [
            'follower_id' => $user->id,
            'following_id' => $target->id,
        ]);

        $this->withHeader('Authorization', "Bearer $token")
            ->deleteJson("/api/unfollow/{$target->id}")
            ->assertStatus(200);

        $this->assertDatabaseMissing('follows', [
            'follower_id' => $user->id,
            'following_id' => $target->id,
        ]);
    }
    public function test_cannot_follow_without_authentication()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();

        $this->postJson("/api/follow/{$target->id}")
            ->assertStatus(401);
    }

    public function test_cannot_follow_self()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $this->withHeader('Authorization', "Bearer $token")
            ->postJson("/api/follow/{$user->id}")
            ->assertStatus(400); // Or your chosen status for this case
    }

    public function test_cannot_follow_same_user_twice()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        // First follow
        $this->withHeader('Authorization', "Bearer $token")
            ->postJson("/api/follow/{$target->id}")
            ->assertStatus(200);

        // Second follow attempt
        $this->withHeader('Authorization', "Bearer $token")
            ->postJson("/api/follow/{$target->id}")
            ->assertStatus(409); // Or your chosen status for duplicate follows
    }

    public function test_unfollow_user_not_followed()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $this->withHeader('Authorization', "Bearer $token")
            ->deleteJson("/api/unfollow/{$target->id}")
            ->assertStatus(404); // Or your chosen status for not found
    }

    public function test_cannot_follow_nonexistent_user()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $this->withHeader('Authorization', "Bearer $token")
            ->postJson("/api/follow/99999")
            ->assertStatus(404);
    }

    public function test_cannot_unfollow_without_authentication()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();

        $this->deleteJson("/api/unfollow/{$target->id}")
            ->assertStatus(401);
    }
    public function test_can_get_user_followers()
    {
        $user = User::factory()->create();
        $follower = User::factory()->create();
        $follower->following()->attach($user->id);

        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/{$user->id}/followers");

        $response->assertStatus(200)
            ->assertJsonFragment(['id' => $follower->id]);
    }
    public function test_can_get_users_currently_following()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();
        $user->following()->attach($target->id);

        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/following");

        $response->assertStatus(200)
            ->assertJsonFragment(['id' => $target->id]);
    }
    public function test_can_check_if_user_is_following_another()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();
        $user->following()->attach($target->id);

        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/isfollowing/{$target->id}");

        $response->assertStatus(200)
            ->assertJson(['isFollowing' => true]);
    }
    public function test_get_followers_of_nonexistent_user()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/99999/followers");

        $response->assertStatus(404);
    }

    public function test_get_followers_when_none_exist()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/{$user->id}/followers");

        $response->assertStatus(200)
            ->assertJsonCount(0, 'followers');
    }

    public function test_get_following_when_none_exist()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/following");

        $response->assertStatus(200)
            ->assertJsonCount(0, 'following');
    }

    public function test_isfollowing_returns_false_when_not_following()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/isfollowing/{$target->id}");

        $response->assertStatus(200)
            ->assertJson(['isFollowing' => false]);
    }

    public function test_isfollowing_returns_false_for_self()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/isfollowing/{$user->id}");

        $response->assertStatus(200)
            ->assertJson(['isFollowing' => false]);
    }

    public function test_following_requires_authentication()
    {
        $response = $this->getJson("/api/following");
        $response->assertStatus(401);
    }

    public function test_isfollowing_requires_authentication()
    {
        $user = User::factory()->create();
        $target = User::factory()->create();

        $response = $this->getJson("/api/isfollowing/{$target->id}");
        $response->assertStatus(401);
    }

    public function test_get_followers_with_invalid_id_format()
    {
        $user = User::factory()->create();
        $token = JWTAuth::fromUser($user);

        $response = $this->withHeader('Authorization', "Bearer $token")
            ->getJson("/api/invalid_id/followers");

        $response->assertStatus(404);
    }
    public function test_can_get_most_followed_users()
    {
        // Create users and simulate followers
        $user1 = User::factory()->create(['number_of_followers' => 5]);
        $user2 = User::factory()->create(['number_of_followers' => 10]);
        $user3 = User::factory()->create(['number_of_followers' => 2]);

        $response = $this->getJson('/api/most_followed');
        $response->assertStatus(200)
            ->assertJsonFragment(['id' => $user2->id])
            ->assertJsonFragment(['id' => $user1->id]);
    }

}
